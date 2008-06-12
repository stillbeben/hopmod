#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#define _dup    dup
#define _fileno fileno
#endif

#include <cubescript.hpp>
#include "hopmod/stopwatch.hpp"
#include "hopmod/textformat.hpp"
#include "hopmod/eventhandler.hpp"
#include "hopmod/script_pipe.hpp"
#include "hopmod/sleep.hpp"
#include "hopmod/module_loader.hpp"
#include "hopmod/wovar.hpp"
#include "hopmod/playerid.hpp"
#include "hopmod/sqlite3.hpp"

#include <boost/bind.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

//defined in engine/server.cpp
extern int g_argc;
extern char * const * g_argv;
extern igameserver * sv;
void cleanupserver();

static void shutdown_from_signal(int);

struct fpsserver : igameserver
{
    struct server_entity            // server side version of "entity" type
    {
        int type;
        int spawntime;
        char spawned;
    };

    static const int DEATHMILLIS = 300;

    enum { GE_NONE = 0, GE_SHOT, GE_EXPLODE, GE_HIT, GE_SUICIDE, GE_PICKUP };

    struct shotevent
    {
        int type;
        int millis, id;
        int gun;
        float from[3], to[3];
    };

    struct explodeevent
    {
        int type;
        int millis, id;
        int gun;
    };

    struct hitevent
    {
        int type;
        int target;
        int lifesequence;
        union
        {
            int rays;
            float dist;
        };
        float dir[3];
    };

    struct suicideevent
    {
        int type;
    };

    struct pickupevent
    {
        int type;
        int ent;
    };

    union gameevent
    {
        int type;
        shotevent shot;
        explodeevent explode;
        hitevent hit;
        suicideevent suicide;
        pickupevent pickup;
    };

    template <int N>
    struct projectilestate
    {
        int projs[N];
        int numprojs;

        projectilestate() : numprojs(0) {}

        void reset() { numprojs = 0; }

        void add(int val)
        {
            if(numprojs>=N) numprojs = 0;
            projs[numprojs++] = val;
        }

        bool remove(int val)
        {
            loopi(numprojs) if(projs[i]==val)
            {
                projs[i] = projs[--numprojs];
                return true;
            }
            return false;
        }
    };
    
    struct gamestate : fpsstate
    {
        vec o;
        int state, editstate;
        int lastdeath, lastspawn, lifesequence;
        int lastshot;
        projectilestate<8> rockets, grenades;
        int frags, deaths, teamkills, shotdamage, damage;

        int lasttimeplayed, timeplayed;
        float effectiveness;

        gamestate() : state(CS_DEAD), editstate(CS_DEAD) {}
    
        bool isalive(int gamemillis)
        {
            return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
        }

        bool waitexpired(int gamemillis)
        {
            return gamemillis - lastshot >= gunwait;
        }

        void reset()
        {
            if(state!=CS_SPECTATOR) state = editstate = CS_DEAD;
            lifesequence = 0;
            maxhealth = 100;
            rockets.reset();
            grenades.reset();

            timeplayed = 0;
            effectiveness = 0;

            frags = deaths = teamkills = shotdamage = damage = 0;
            
            respawn();
        }

        void respawn()
        {
            fpsstate::respawn();
            o = vec(-1e10f, -1e10f, -1e10f);
            lastdeath = 0;
            lastspawn = -1;
            lastshot = 0;
        }
    };

    struct savedscore
    {
        uint ip;
        string name;
        int maxhealth, frags, deaths, teamkills, shotdamage, damage;
        int timeplayed;
        float effectiveness;

        void save(gamestate &gs)
        {
            maxhealth = gs.maxhealth;
            frags = gs.frags;
            deaths = gs.deaths;
            teamkills = gs.teamkills;
            shotdamage = gs.shotdamage;
            damage = gs.damage;
            timeplayed = gs.timeplayed;
            effectiveness = gs.effectiveness;
        }

        void restore(gamestate &gs)
        {
            if(gs.health==gs.maxhealth) gs.health = maxhealth;
            gs.maxhealth = maxhealth;
            gs.frags = frags;
            gs.deaths = deaths;
            gs.teamkills = teamkills;
            gs.shotdamage = shotdamage;
            gs.damage = damage;
            gs.timeplayed = timeplayed;
            gs.effectiveness = effectiveness;
        }
    };

    struct clientinfo
    {
        int clientnum;
        string name, team, mapvote;
        int modevote;
        int privilege; bool hidden_priv;
        bool spectator, local, timesync, wantsmaster;
        int gameoffset, lastevent;
        gamestate state;
        vector<gameevent> events;
        vector<uchar> position, messages;
        vector<clientinfo *> targets;
        
        stopwatch svtext_interval;
        stopwatch svsetmaster_interval;
        stopwatch svkick_interval;
        stopwatch svmapvote_interval;
        
        bool connected;
        int disc_reason;
        int connect_time;
        int connect_id;
        int ping;
        int lastupdate;
        int lag;
        bool ban;
        int bantime;
        
        enum var_type
        {
            GAME_VAR=0,    //exists for duration of current game only
            PERM_VAR,     //permanent variable
        };
        typedef std::map<std::string,std::pair<var_type,std::string> > varmap;
        //varmap m_vars;
        //varmap::iterator m_find_cache;
        
        clientinfo()
         :hidden_priv(false),connected(false),disc_reason(0),ban(false)
        { 
            reset();
        }

        gameevent &addevent()
        {
            static gameevent dummy;
            if(state.state==CS_SPECTATOR || events.length()>100) return dummy;
            return events.add();
        }

        void mapchange()
        {
            mapvote[0] = 0;
            state.reset();
            events.setsizenodelete(0);
            targets.setsizenodelete(0);
            timesync = false;
            lastevent = 0;
        }

        void reset()
        {
            name[0] = team[0] = 0;
            privilege = PRIV_NONE;
            spectator = local = wantsmaster = false;
            position.setsizenodelete(0);
            messages.setsizenodelete(0);
            mapchange();
            ping=0;
            lastupdate=-1;
            lag=0;
        }
        
        void sendprivmsg(const char * msg)
        {
            sendf(clientnum, 1, "ris", SV_SERVMSG, msg);
        }
        
        bool check_svtext_flooding(stopwatch::milliseconds min_interval)
        {
            bool flooding=false;
            
            if( svtext_interval.running() && svtext_interval.stop().get_elapsed() < min_interval )
            {
                std::ostringstream block_msg;
                block_msg<<ConColour_Error<<"You are sending too many messages at once!";
                sendprivmsg(block_msg.str().c_str());
                svtext_interval.resume();
                flooding=true;
            }
            else svtext_interval.start();
            
            return flooding;
        }
        
        bool check_flooding(stopwatch & sw,stopwatch::milliseconds min_interval,const char * what)
        {
            bool flooding=false;
            
            if( sw.running() && sw.stop().get_elapsed() < min_interval )
            {
                int time_left = (min_interval - sw.get_elapsed())/1000;
                std::ostringstream block_msg;
                block_msg<<ConColour_Error<<"You are blocked from "<<what<<" for "<<time_left<<" secs.";
                sendprivmsg(block_msg.str().c_str());
                sw.resume();
                flooding = true;
            }
            else sw.start();
            
            return flooding;
        }
        
        playerid id()const
        {
            return playerid(std::string(name),getclientip(clientnum));
        }
    };

    struct worldstate
    {
        int uses;
        vector<uchar> positions, messages;
    };

    struct ban
    {
        ban():time(0),expire(0),ip(0){}
        ban(int t,int e,uint i):time(t),expire(e),ip(i){}
        int time;
        int expire;
        uint ip;
    };
    
    int pending_bans;
    
    #define MM_MODE 0xF
    #define MM_AUTOAPPROVE 0x1000
    #define MM_DEFAULT (MM_MODE)

    enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE };
 
    bool notgotitems, notgotbases;        // true when map has changed and waiting for clients to send item
    int gamemode; string sgamemode;
    int gamemillis, gamelimit;
    int gamecount;
    int playercount;
    int concount;
    
    string serverdesc;
    string smapname;
    int lastmillis, totalmillis, curtime;
    int interm, minremain;
    bool mapreload;
    enet_uint32 lastsend;
    int mastermode, mastermask;
    int currentmaster;
    bool masterupdate;
    string masterpass;
    FILE *mapdata;

    vector<uint> allowedips;
    vector<ban> bannedips;
    vector<clientinfo *> clients;
    vector<worldstate *> worldstates;
    bool reliablemessages;
    
    struct demofile
    {
        string info;
        uchar *data;
        int len;
    };

    #define MAXDEMOS 5
    vector<demofile> demos;

    bool demonextmatch;
    FILE *demotmp;
    gzFile demorecord, demoplayback;
    int nextplayback;
    std::string demofilename;
    
    struct servmode
    {
        fpsserver &sv;

        servmode(fpsserver &sv) : sv(sv) {}
        virtual ~servmode() {}

        virtual void entergame(clientinfo *ci) {}
        virtual void leavegame(clientinfo *ci, bool disconnecting = false) {}

        virtual void moved(clientinfo *ci, const vec &oldpos, const vec &newpos) {}
        virtual bool canspawn(clientinfo *ci, bool connecting = false) { return true; }
        virtual void spawned(clientinfo *ci) {}
        virtual int fragvalue(clientinfo *victim, clientinfo *actor)
        {
            int gamemode = sv.gamemode;
            if(victim==actor || isteam(victim->team, actor->team)) return -1;
            return 1;
        }
        virtual void died(clientinfo *victim, clientinfo *actor) {}
        virtual bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam) { return true; }
        virtual void changeteam(clientinfo *ci, const char *oldteam, const char *newteam) {}
        virtual void initclient(clientinfo *ci, ucharbuf &p, bool connecting) {}
        virtual void update() {}
        virtual void reset(bool empty) {}
        virtual void intermission() {}
    };

    struct arenaservmode : servmode
    {
        int arenaround;

        arenaservmode(fpsserver &sv) : servmode(sv), arenaround(0) {}

        bool canspawn(clientinfo *ci, bool connecting = false) 
        { 
            if(connecting && sv.nonspectators(ci->clientnum)<=1) return true;
            return false; 
        }

        void reset(bool empty)
        {
            arenaround = 0;
        }
    
        void update()
        {
            if(sv.interm || sv.gamemillis<arenaround || !sv.nonspectators()) return;
    
            if(arenaround)
            {
                arenaround = 0;
                loopv(sv.clients) if(sv.clients[i]->state.state==CS_DEAD || sv.clients[i]->state.state==CS_ALIVE) 
                {
                    sv.clients[i]->state.respawn();
                    sv.sendspawn(sv.clients[i]);
                }
                return;
            }

            int gamemode = sv.gamemode;
            clientinfo *alive = NULL;
            bool dead = false;
            loopv(sv.clients)
            {
                clientinfo *ci = sv.clients[i];
                if(ci->state.state==CS_ALIVE || (ci->state.state==CS_DEAD && ci->state.lastspawn>=0))
                {
                    if(!alive) alive = ci;
                    else if(!m_teammode || strcmp(alive->team, ci->team)) return;
                }
                else if(ci->state.state==CS_DEAD) dead = true;
            }
            if(!dead) return;
            sendf(-1, 1, "ri2", SV_ARENAWIN, !alive ? -1 : alive->clientnum);
            arenaround = sv.gamemillis+5000;
        }
    };

    #define CAPTURESERV 1
    #include "capture.h"
    #undef CAPTURESERV

    #define ASSASSINSERV 1
    #include "assassin.h"
    #undef ASSASSINSERV

    #define CTFSERV 1
    #include "ctf.h"
    #undef CTFSERV

    arenaservmode arenamode;
    captureservmode capturemode;
    assassinservmode assassinmode;
    ctfservmode ctfmode;
    servmode *smode;
    
    cubescript::domain server_domain;
    
    cubescript::function2<void_,const std::string &,int>    func_flood_protection;
    cubescript::function1<void_,const std::string &>        func_log_status;
    cubescript::function1<void_,const std::string &>        func_log_error;
    cubescript::function1<void_,const std::string &>        func_msg;
    cubescript::function2<void_,int,const std::string &>    func_privmsg;
    cubescript::function1<std::string,int>                  func_player_name;
    cubescript::function1<std::string,int>                  func_player_ip;
    cubescript::function1<std::string,int>                  func_player_team;
    cubescript::function1<std::string,int>                  func_player_status;
    cubescript::function1<int,int>                          func_player_contime;
    cubescript::function1<int,int>                          func_player_conid;
    cubescript::function1<std::string,int>                  func_player_priv;
    cubescript::function1<int,int>                          func_player_frags;
    cubescript::function1<std::string,int>                  func_player_gun;
    cubescript::function1<int,int>                          func_player_health;
    cubescript::function1<int,int>                          func_player_maxhealth;
    cubescript::functionV<std::string>                      func_player_var;
    cubescript::functionV<std::string>                      func_player_pvar;
    cubescript::function2<bool,int,const std::string &>     func_player_has_var;
    cubescript::function1<int,int>                          func_player_ping;
    cubescript::function1<int,int>                          func_player_lag;
    cubescript::function1<std::string,int>                  func_get_disc_reason;
    cubescript::function2<void_,int,const std::string &>    func_setpriv;
    cubescript::function0<void_>                            func_clearbans;
    cubescript::functionV<void_>                            func_kick;
    cubescript::function1<void_,int>                        func_set_interm;
    cubescript::function1<void_,int>                        func_spec;
    cubescript::function1<void_,int>                        func_unspec;
    cubescript::function0<std::vector<int> >                func_players;
    cubescript::function2<void_,int,bool>                   func_setmaster;
    cubescript::function1<void_,int>                        func_approve;
    cubescript::function2<void_,std::string,std::string>    func_changemap;
    cubescript::function2<void_,bool,std::string>           func_recorddemo;
    cubescript::function0<void_>                            func_stopdemo;
    cubescript::function1<void_,const std::string &>        func_allowhost;
    cubescript::function1<void_,const std::string &>        func_denyhost;
    cubescript::function1<int,const std::string &>          func_capture_score;
    cubescript::function1<std::string,const std::string &>  func_shell_quote;
    cubescript::function0<bool>                             func_teamgame;
    cubescript::function0<bool>                             func_itemsgame;
    cubescript::function0<bool>                             func_capturegame;
    cubescript::function0<bool>                             func_ctfgame;
    cubescript::function1<std::string,const std::string &>  func_nsresolve;
    cubescript::function1<bool,int>                         func_dupname;
    cubescript::function0<void_>                            func_shutdown;
    cubescript::function0<void_>                            func_restarter;
    cubescript::function2<void_,std::string,std::string>    func_logfile;
    cubescript::function4<pid_t,const std::string &,
                                const std::vector<std::string> &,
                                const std::string &,
                                const std::string & >       func_daemon;
    cubescript::function1<void_,pid_t>                      func_kill; //TODO move to libcubescript system runtime
    cubescript::function1<void_,int>                        func_server_sleep;
    
    cubescript::variable_ref<int>                           var_maxclients;
    cubescript::variable_ref<int>                           var_mastermode;
    cubescript::variable_ref<int>                           var_gametime;
    cubescript::cstr_variable                               var_mapname;
    cubescript::cstr_variable                               var_title;
    cubescript::wo_cstr_variable                            var_password;
    cubescript::cstr_variable                               var_gamemode;
    cubescript::variable_ref<int>                           var_gamecount;
    cubescript::variable_ref<int>                           var_playercount;
    cubescript::variable_ref<int>                           var_uptime;
    cubescript::variable_ref<int>                           var_timeleft;
    
    cubescript::constant<int>                               const_mm_open;
    cubescript::constant<int>                               const_mm_veto;
    cubescript::constant<int>                               const_mm_locked;
    cubescript::constant<int>                               const_mm_private;
    
    script_pipe_service m_script_pipes;
    sleep_service m_sleep_jobs;
    //cubescript::module_loader m_modules;
    std::list<FILE *> m_logfiles;
    
    stopwatch::milliseconds svtext_min_interval;
    stopwatch::milliseconds svsetmaster_min_interval;
    stopwatch::milliseconds svkick_min_interval;
    stopwatch::milliseconds svmapvote_min_interval;
    
    event_handler_service scriptable_events;
    event_handler on_startup;
    event_handler on_shutdown;
    event_handler on_text;
    event_handler on_connect;
    event_handler on_disconnect;
    event_handler on_kick;
    event_handler on_mapvote;
    event_handler on_mapchange;
    event_handler on_setmap;
    event_handler on_rename;
    event_handler on_reteam;
    event_handler on_chmm;
    event_handler on_newmap;
    event_handler on_setmaster;
    event_handler on_approvemaster;
    event_handler on_spectator;
    event_handler on_death;
    event_handler on_timeupdate;
    event_handler on_itempickup;
    event_handler on_damage;
    
    fpsserver() : pending_bans(0), notgotitems(true), notgotbases(false), gamemode(0), gamecount(0), playercount(0), concount(0), interm(0), minremain(0), mapreload(false), lastsend(0), mastermode(MM_OPEN), mastermask(MM_DEFAULT), currentmaster(-1), masterupdate(false), mapdata(NULL), reliablemessages(false), demonextmatch(false), demotmp(NULL), demorecord(NULL), demoplayback(NULL), nextplayback(0), arenamode(*this), capturemode(*this), assassinmode(*this), ctfmode(*this), smode(NULL), 
        
        func_flood_protection(boost::bind(&fpsserver::set_flood_protection,this,_1,_2)),
        func_log_status(boost::bind(&fpsserver::log_status,this,_1)),
        func_log_error(boost::bind(&fpsserver::log_error,this,_1)),
        func_msg(boost::bind(&fpsserver::send_msg,this,_1)),
        func_privmsg(boost::bind(&fpsserver::send_privmsg,this,_1,_2)),
        func_player_name(boost::bind(&fpsserver::get_player_name,this,_1)),
        func_player_ip(boost::bind(&fpsserver::get_player_ip,this,_1)),
        func_player_team(boost::bind(&fpsserver::get_player_team,this,_1)),
        func_player_status(boost::bind(&fpsserver::get_player_status,this,_1)),
        func_player_contime(boost::bind(&fpsserver::get_player_contime,this,_1)),
        func_player_conid(boost::bind(&fpsserver::get_player_conid,this,_1)),
        func_player_priv(boost::bind(&fpsserver::get_player_priv,this,_1)),
        func_player_frags(boost::bind(&fpsserver::get_player_frags,this,_1)),
        func_player_gun(boost::bind(&fpsserver::get_player_gun,this,_1)),
        func_player_health(boost::bind(&fpsserver::get_player_health,this,_1)),
        func_player_maxhealth(boost::bind(&fpsserver::get_player_maxhealth,this,_1)),
        func_player_var(boost::bind(&fpsserver::access_player_var,this,_1,_2,clientinfo::GAME_VAR)),
        func_player_pvar(boost::bind(&fpsserver::access_player_var,this,_1,_2,clientinfo::PERM_VAR)),
        func_player_has_var(boost::bind(&fpsserver::has_player_var,this,_1,_2)),
        func_player_ping(boost::bind(&fpsserver::get_player_ping,this,_1)),
        func_player_lag(boost::bind(&fpsserver::get_player_lag,this,_1)),
        func_get_disc_reason(boost::bind(&fpsserver::get_disc_reason,this,_1)),
        func_setpriv(boost::bind(&fpsserver::setpriv,this,_1,_2)),
        func_clearbans(boost::bind(&fpsserver::clearbans,this)),
        func_kick(boost::bind(&fpsserver::kickban,this,_1,_2)),
        func_set_interm(boost::bind(&fpsserver::set_interm,this,_1)),
        func_spec(boost::bind(&fpsserver::set_spectator,this,_1,true)),
        func_unspec(boost::bind(&fpsserver::set_spectator,this,_1,false)),
        func_players(boost::bind(&fpsserver::players,this)),
        func_setmaster(boost::bind(&fpsserver::console_setmaster,this,_1,_2)),
        func_approve(boost::bind(&fpsserver::approvemaster,this,_1)),
        func_changemap(boost::bind(&fpsserver::changemap_,this,_1,_2)),
        func_recorddemo(boost::bind(&fpsserver::recorddemo,this,_1,_2)),
        func_stopdemo(boost::bind(&fpsserver::stopdemo,this)),
        func_allowhost(boost::bind(&fpsserver::add_allowhost,this,_1)),
        func_denyhost(boost::bind(&fpsserver::add_denyhost,this,_1)),
        func_capture_score(boost::bind(&fpsserver::get_capture_score,this,_1)),
        func_shell_quote(boost::bind(&fpsserver::shell_quote,this,_1)),
        func_teamgame(boost::bind(&fpsserver::is_teamgame,this)),
        func_itemsgame(boost::bind(&fpsserver::is_itemsgame,this)),
        func_capturegame(boost::bind(&fpsserver::is_capturegame,this)),
        func_ctfgame(boost::bind(&fpsserver::is_ctfgame,this)),
        func_nsresolve(boost::bind(&fpsserver::nsresolve,this,_1)),
        func_dupname(boost::bind(&fpsserver::duplicatename,this,_1)),
        func_shutdown(boost::bind(&fpsserver::shutdown,this)),
        func_restarter(boost::bind(&fpsserver::create_restarter,this)),
        func_logfile(boost::bind(&fpsserver::create_logfile_function,this,_1,_2)),
        func_daemon(boost::bind(&fpsserver::spawn_daemon,this,_1,_2,_3,_4)),
        func_kill(boost::bind(&fpsserver::kill_process,this,_1)),
        func_server_sleep(boost::bind(&fpsserver::server_sleep,this,_1)),
        
        var_maxclients(maxclients),
        var_mastermode(mastermode),
        var_gametime(gamelimit),
        var_mapname(smapname,sizeof(smapname)),
        var_title(serverdesc,sizeof(serverdesc)),
        var_password(masterpass,sizeof(masterpass)),
        var_gamemode(sgamemode,sizeof(sgamemode)),
        var_gamecount(gamecount),
        var_playercount(playercount),
        var_uptime(totalmillis),
        var_timeleft(minremain),
        
        const_mm_open(MM_OPEN),
        const_mm_veto(MM_VETO),
        const_mm_locked(MM_LOCKED),
        const_mm_private(MM_PRIVATE),
        
        m_script_pipes(std::cerr),
        
        svtext_min_interval(0), 
        svsetmaster_min_interval(0),
        svkick_min_interval(0),
        svmapvote_min_interval(0),
        
        scriptable_events(&server_domain)
    {
        serverdesc[0] = '\0';
        masterpass[0] = '\0';
        sgamemode[0]='\0';
        
        cubescript::runtime::register_core_functions(&server_domain);
        cubescript::runtime::register_system_functions(&server_domain);
        
        server_domain.register_symbol("flood_protection",&func_flood_protection);
        server_domain.register_symbol("log_status",&func_log_status);
        server_domain.register_symbol("log_error",&func_log_error);
        server_domain.register_symbol("msg",&func_msg);
        server_domain.register_symbol("privmsg",&func_privmsg);
        server_domain.register_symbol("player_name",&func_player_name);
        server_domain.register_symbol("player_ip",&func_player_ip);
        server_domain.register_symbol("player_team",&func_player_team);
        server_domain.register_symbol("player_status",&func_player_status);
        server_domain.register_symbol("player_contime",&func_player_contime);
        server_domain.register_symbol("player_conid",&func_player_conid);
        server_domain.register_symbol("player_priv",&func_player_priv);
        server_domain.register_symbol("player_frags",&func_player_frags);
        server_domain.register_symbol("player_gun",&func_player_gun);
        server_domain.register_symbol("player_health",&func_player_health);
        server_domain.register_symbol("player_maxhealth",&func_player_maxhealth);
        server_domain.register_symbol("player_var",&func_player_var);
        server_domain.register_symbol("player_pvar",&func_player_pvar);
        server_domain.register_symbol("player_has_var",&func_player_has_var);
        server_domain.register_symbol("player_ping",&func_player_ping);
        server_domain.register_symbol("player_lag",&func_player_lag);
        server_domain.register_symbol("disc_reason",&func_get_disc_reason);
        server_domain.register_symbol("setpriv",&func_setpriv);
        server_domain.register_symbol("clearbans",&func_clearbans);
        server_domain.register_symbol("kick",&func_kick);
        server_domain.register_symbol("intermission",&func_set_interm);
        server_domain.register_symbol("spec",&func_spec);
        server_domain.register_symbol("unspec",&func_unspec);
        server_domain.register_symbol("players",&func_players);
        server_domain.register_symbol("setmaster",&func_setmaster);
        server_domain.register_symbol("approve",&func_approve);
        server_domain.register_symbol("changemap",&func_changemap);
        server_domain.register_symbol("recorddemo",&func_recorddemo);
        server_domain.register_symbol("stopdemo",&func_stopdemo);
        server_domain.register_symbol("allowhost",&func_allowhost);
        server_domain.register_symbol("denyhost",&func_denyhost);
        server_domain.register_symbol("shell_quote",&func_shell_quote);
        server_domain.register_symbol("teamgame",&func_teamgame);
        server_domain.register_symbol("itemsgame",&func_itemsgame);
        server_domain.register_symbol("capturegame",&func_capturegame);
        server_domain.register_symbol("ctfgame",&func_ctfgame);
        server_domain.register_symbol("resolve",&func_nsresolve);
        server_domain.register_symbol("dupname",&func_dupname);
        server_domain.register_symbol("shutdown",&func_shutdown);
        server_domain.register_symbol("restarter",&func_restarter);
        server_domain.register_symbol("logfile",&func_logfile);
        server_domain.register_symbol("daemon",&func_daemon);
        server_domain.register_symbol("kill",&func_kill);
        server_domain.register_symbol("server_sleep",&func_server_sleep);
        
        server_domain.register_symbol("maxclients",&var_maxclients);
        server_domain.register_symbol("mastermode",&var_mastermode);
        server_domain.register_symbol("gametime",&var_gametime);
        server_domain.register_symbol("mapname",&var_mapname); var_mapname.readonly(true);
        server_domain.register_symbol("title",&var_title);
        server_domain.register_symbol("password",&var_password);
        server_domain.register_symbol("gamemode",&var_gamemode); var_gamemode.readonly(true);
        server_domain.register_symbol("gamecount",&var_gamecount);
        server_domain.register_symbol("playercount",&var_playercount); var_playercount.readonly(true);
        server_domain.register_symbol("uptime",&var_uptime);
        server_domain.register_symbol("timeleft",&var_timeleft); var_timeleft.readonly(true);
        
        server_domain.register_symbol("MM_OPEN",&const_mm_open);
        server_domain.register_symbol("MM_VETO",&const_mm_veto);
        server_domain.register_symbol("MM_LOCKED",&const_mm_locked);
        server_domain.register_symbol("MM_PRIVATE",&const_mm_private);
        
        cubescript::bind(g_argv[0],"SERVER_FILENAME",&server_domain);
        
        m_script_pipes.register_function(&server_domain);
        m_sleep_jobs.register_function(&server_domain);
        cubescript::register_module_loader(&server_domain);
    #ifdef USE_SQLITE3
        cubescript::register_sqlite3(&server_domain);
    #endif
        
        scriptable_events.register_event("onstartup",&on_startup);
        scriptable_events.register_event("onshutdown",&on_shutdown);
        scriptable_events.register_event("ontext",&on_text);
        scriptable_events.register_event("onconnect",&on_connect);
        scriptable_events.register_event("ondisconnect",&on_disconnect);
        scriptable_events.register_event("onkick",&on_kick);
        scriptable_events.register_event("onmapvote",&on_mapvote);
        scriptable_events.register_event("onmapchange",&on_mapchange);
        scriptable_events.register_event("onsetmap",&on_setmap);
        scriptable_events.register_event("onrename",&on_rename);
        scriptable_events.register_event("onreteam",&on_reteam);
        scriptable_events.register_event("onchmm",&on_chmm);
        scriptable_events.register_event("onnewmap",&on_newmap);
        scriptable_events.register_event("onsetmaster",&on_setmaster);
        scriptable_events.register_event("onapprovemaster",&on_approvemaster);
        scriptable_events.register_event("onspectator",&on_spectator);
        scriptable_events.register_event("ondeath",&on_death);
        scriptable_events.register_event("ontimeupdate",&on_timeupdate);
        scriptable_events.register_event("onitempickup",&on_itempickup);
        scriptable_events.register_event("ondamage",&on_damage);
    }
    
    void *newinfo() { return new clientinfo; }
    void deleteinfo(void *ci) { delete (clientinfo *)ci; }
    
    inline clientinfo * get_ci(int cn)
    {
        clientinfo * ci=(clientinfo *)getinfo(cn);
        if(!ci) throw cubescript::error_key("runtime.function.get_clientinfo.invalid_cn");
        return ci;
    }
    
    vector<server_entity> sents;
    vector<savedscore> scores;
    player_map<clientinfo::varmap> vars;
    
    static const char *modestr(int n, const char *unknown = "unknown")
    {
        static const char *modenames[] =
        {
            "slowmo SP", "slowmo DMSP", "demo", "SP", "DMSP", "ffa/default", "coopedit", "ffa/duel", "teamplay",
            "instagib", "instagib team", "efficiency", "efficiency team",
            "insta arena", "insta clan arena", "tactics arena", "tactics clan arena",
            "capture", "insta capture", "regen capture", "assassin", "insta assassin",
            "ctf", "insta ctf"
        };
        return (n>=-5 && size_t(n+5)<sizeof(modenames)/sizeof(modenames[0])) ? modenames[n+5] : unknown;
    }
    
    static const char *mastermodestr(int n, const char *unknown = "unknown")
    {
        static const char *mastermodenames[] =
        {
            "open", "veto", "locked", "private"
        };
        return (n>=0 && size_t(n)<sizeof(mastermodenames)/sizeof(mastermodenames[0])) ? mastermodenames[n] : unknown;
    }
    
    static int modecode(const char * str)
    {
        for(int i=0; i<18; i++) if(strcmp(modestr(i),str)==0) return i;
        return -1;
    }
    
    void sendservmsg(const char *s) { sendf(-1, 1, "ris", SV_SERVMSG, s); }

    void resetitems() 
    { 
        sents.setsize(0);
        //cps.reset(); 
    }

    int spawntime(int type)
    {
        if(m_classicsp) return INT_MAX;
        int np = nonspectators();
        np = np<3 ? 4 : (np>4 ? 2 : 3);         // spawn times are dependent on number of players
        int sec = 0;
        switch(type)
        {
            case I_SHELLS:
            case I_BULLETS:
            case I_ROCKETS:
            case I_ROUNDS:
            case I_GRENADES:
            case I_CARTRIDGES: sec = np*4; break;
            case I_HEALTH: sec = np*5; break;
            case I_GREENARMOUR:
            case I_YELLOWARMOUR: sec = 20; break;
            case I_BOOST:
            case I_QUAD: sec = 40+rnd(40); break;
        }
        return sec*1000;
    }
    
    std::string itemname(int item)
    {
        static const char * names[]={"shells","bullets","rockets",
            "rounds","grenades","cartridges","health","boost","greenarmor",
            "yellowarmor","quad"};
        return names[item-I_SHELLS];
    }
    
    bool pickup(int i, int sender)         // server side item pickup, acknowledge first client that gets it
    {
        if(minremain<=0 || !sents.inrange(i) || !sents[i].spawned) return false;
        clientinfo *ci = (clientinfo *)getinfo(sender);
        if(!ci || (!ci->local && !ci->state.canpickup(sents[i].type))) return false;
        sents[i].spawned = false;
        sents[i].spawntime = spawntime(sents[i].type);
        sendf(-1, 1, "ri3", SV_ITEMACC, i, sender);
        ci->state.pickup(sents[i].type);
        
        cubescript::arguments args;
        scriptable_events.dispatch(&on_itempickup,args & sender & itemname(sents[i].type),NULL);
        return true;
    }
    
    void vote(char *map, int reqmode, int sender)
    {
        clientinfo *ci = (clientinfo *)getinfo(sender);
        if(!ci || (ci->state.state==CS_SPECTATOR && !ci->privilege)) return;
        s_strcpy(ci->mapvote, map);
        ci->modevote = reqmode;
        if(!ci->mapvote[0]) return;
        if(ci->local || mapreload || (ci->privilege && mastermode>=MM_VETO))
        {
            if(demorecord) enddemorecord();
            if(!ci->local && !mapreload) 
            {
                s_sprintfd(msg)("%s forced %s on map %s", privname(ci->privilege), modestr(reqmode), map);
                sendservmsg(msg);
            }
            sendf(-1, 1, "risii", SV_MAPCHANGE, ci->mapvote, ci->modevote, 1);
            changemap(ci->mapvote, ci->modevote,gamelimit);
        }
        else 
        {
            s_sprintfd(msg)("%s suggests %s on map %s (select map to vote)", colorname(ci), modestr(reqmode), map);
            sendservmsg(msg);
            checkvotes();
        }
    }

    clientinfo *choosebestclient(float &bestrank)
    {
        clientinfo *best = NULL;
        bestrank = -1;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.timeplayed<0) continue;
            float rank = ci->state.state!=CS_SPECTATOR ? ci->state.effectiveness/max(ci->state.timeplayed, 1) : -1;
            if(!best || rank > bestrank) { best = ci; bestrank = rank; }
        }
        return best;
    }

    void autoteam()
    {
        static const char *teamnames[2] = {"good", "evil"};
        vector<clientinfo *> team[2];
        float teamrank[2] = {0, 0};
        for(int round = 0, remaining = clients.length(); remaining>=0; round++)
        {
            int first = round&1, second = (round+1)&1, selected = 0;
            while(teamrank[first] <= teamrank[second])
            {
                float rank;
                clientinfo *ci = choosebestclient(rank);
                if(!ci) break;
                if(m_capture || m_ctf) rank = 1;
                else if(selected && rank<=0) break;    
                ci->state.timeplayed = -1;
                team[first].add(ci);
                if(rank>0) teamrank[first] += rank;
                selected++;
                if(rank<=0) break;
            }
            if(!selected) break;
            remaining -= selected;
        }
        loopi(sizeof(team)/sizeof(team[0]))
        {
            loopvj(team[i])
            {
                clientinfo *ci = team[i][j];
                if(!strcmp(ci->team, teamnames[i])) continue;
                s_strncpy(ci->team, teamnames[i], MAXTEAMLEN+1);
                sendf(-1, 1, "riis", SV_SETTEAM, ci->clientnum, teamnames[i]);
            }
        }
    }

    struct teamrank
    {
        const char *name;
        float rank;
        int clients;

        teamrank(const char *name) : name(name), rank(0), clients(0) {}
    };
    
    const char *chooseworstteam(const char *suggest = NULL, clientinfo *exclude = NULL)
    {
        teamrank teamranks[2] = { teamrank("good"), teamrank("evil") };
        const int numteams = sizeof(teamranks)/sizeof(teamranks[0]);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci==exclude || ci->state.state==CS_SPECTATOR || !ci->team[0]) continue;
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
            ci->state.lasttimeplayed = lastmillis;

            loopj(numteams) if(!strcmp(ci->team, teamranks[j].name)) 
            { 
                teamrank &ts = teamranks[j];
                ts.rank += ci->state.effectiveness/max(ci->state.timeplayed, 1);
                ts.clients++;
                break;
            }
        }
        teamrank *worst = &teamranks[numteams-1];
        loopi(numteams-1)
        {
            teamrank &ts = teamranks[i];
            if(m_capture || m_ctf)
            {
                if(ts.clients < worst->clients || (ts.clients == worst->clients && ts.rank < worst->rank)) worst = &ts;
            }
            else if(ts.rank < worst->rank || (ts.rank == worst->rank && ts.clients < worst->clients)) worst = &ts;
        }
        return worst->name;
    }

    void writedemo(int chan, void *data, int len)
    {
        if(!demorecord) return;
        int stamp[3] = { gamemillis, chan, len };
        endianswap(stamp, sizeof(int), 3);
        gzwrite(demorecord, stamp, sizeof(stamp));
        gzwrite(demorecord, data, len);
    }

    void recordpacket(int chan, void *data, int len)
    {
        writedemo(chan, data, len);
    }

    void enddemorecord()
    {
        if(!demorecord) return;

        gzclose(demorecord);
        demorecord = NULL;

#ifdef WIN32
        demotmp = fopen("demorecord", "rb");
#endif    
        if(!demotmp) return;

        fseek(demotmp, 0, SEEK_END);
        int len = ftell(demotmp);
        rewind(demotmp);
        if(demos.length()>=MAXDEMOS)
        {
            delete[] demos[0].data;
            demos.remove(0);
        }
        demofile &d = demos.add();
        time_t t = time(NULL);
        char *timestr = ctime(&t), *trim = timestr + strlen(timestr);
        while(trim>timestr && isspace(*--trim)) *trim = '\0';
        s_sprintf(d.info)("%s: %s, %s, %.2f%s", timestr, modestr(gamemode), smapname, len > 1024*1024 ? len/(1024*1024.f) : len/1024.0f, len > 1024*1024 ? "MB" : "kB");
        s_sprintfd(msg)("demo \"%s\" recorded", d.info);
        sendservmsg(msg);
        d.data = new uchar[len];
        d.len = len;
        fread(d.data, 1, len, demotmp);
        fclose(demotmp);
        demotmp = NULL;
    }

    void setupdemorecord()
    {
        if(haslocalclients() || !m_mp(gamemode) || gamemode==1) return;

#ifdef WIN32
        gzFile f = gzopen("demorecord", "wb9");
        if(!f) return;
#else
        demotmp = demofilename.length() ? fopen(demofilename.c_str(),"w") : tmpfile();
        if(!demotmp) return;
        setvbuf(demotmp, NULL, _IONBF, 0);

        gzFile f = gzdopen(_dup(_fileno(demotmp)), "wb9");
        if(!f)
        {
            fclose(demotmp);
            demotmp = NULL;
            return;
        }
#endif

        sendservmsg("recording demo");

        demorecord = f;

        demoheader hdr;
        memcpy(hdr.magic, DEMO_MAGIC, sizeof(hdr.magic));
        hdr.version = DEMO_VERSION;
        hdr.protocol = PROTOCOL_VERSION;
        endianswap(&hdr.version, sizeof(int), 1);
        endianswap(&hdr.protocol, sizeof(int), 1);
        gzwrite(demorecord, &hdr, sizeof(demoheader));

        ENetPacket *packet = enet_packet_create(NULL, MAXTRANS, 0);
        ucharbuf p(packet->data, packet->dataLength);
        welcomepacket(p, -1, packet);
        writedemo(1, p.buf, p.len);
        enet_packet_destroy(packet);

        uchar buf[MAXTRANS];
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            uchar header[16];
            ucharbuf q(&buf[sizeof(header)], sizeof(buf)-sizeof(header));
            putint(q, SV_INITC2S);
            sendstring(ci->name, q);
            sendstring(ci->team, q);

            ucharbuf h(header, sizeof(header));
            putint(h, SV_CLIENT);
            putint(h, ci->clientnum);
            putuint(h, q.len);

            memcpy(&buf[sizeof(header)-h.len], header, h.len);

            writedemo(1, &buf[sizeof(header)-h.len], h.len+q.len);
        }
    }

    void listdemos(int cn)
    {
        ENetPacket *packet = enet_packet_create(NULL, MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        if(!packet) return;
        ucharbuf p(packet->data, packet->dataLength);
        putint(p, SV_SENDDEMOLIST);
        putint(p, demos.length());
        loopv(demos) sendstring(demos[i].info, p);
        enet_packet_resize(packet, p.length());
        sendpacket(cn, 1, packet);
        if(!packet->referenceCount) enet_packet_destroy(packet);
    }

    void cleardemos(int n)
    {
        if(!n)
        {
            loopv(demos) delete[] demos[i].data;
            demos.setsize(0);
            sendservmsg("cleared all demos");
        }
        else if(demos.inrange(n-1))
        {
            delete[] demos[n-1].data;
            demos.remove(n-1);
            s_sprintfd(msg)("cleared demo %d", n);
            sendservmsg(msg);
        }
    }

    void senddemo(int cn, int num)
    {
        if(!num) num = demos.length();
        if(!demos.inrange(num-1)) return;
        demofile &d = demos[num-1];
        sendf(cn, 2, "rim", SV_SENDDEMO, d.len, d.data); 
    }

    void setupdemoplayback()
    {
        demoheader hdr;
        string msg;
        msg[0] = '\0';
        s_sprintfd(file)("%s.dmo", smapname);
        demoplayback = opengzfile(file, "rb9");
        if(!demoplayback) s_sprintf(msg)("could not read demo \"%s\"", file);
        else if(gzread(demoplayback, &hdr, sizeof(demoheader))!=sizeof(demoheader) || memcmp(hdr.magic, DEMO_MAGIC, sizeof(hdr.magic)))
            s_sprintf(msg)("\"%s\" is not a demo file", file);
        else 
        { 
            endianswap(&hdr.version, sizeof(int), 1);
            endianswap(&hdr.protocol, sizeof(int), 1);
            if(hdr.version!=DEMO_VERSION) s_sprintf(msg)("demo \"%s\" requires an %s version of Sauerbraten", file, hdr.version<DEMO_VERSION ? "older" : "newer");
            else if(hdr.protocol!=PROTOCOL_VERSION) s_sprintf(msg)("demo \"%s\" requires an %s version of Sauerbraten", file, hdr.protocol<PROTOCOL_VERSION ? "older" : "newer");
        }
        if(msg[0])
        {
            if(demoplayback) { gzclose(demoplayback); demoplayback = NULL; }
            sendservmsg(msg);
            return;
        }

        s_sprintf(msg)("playing demo \"%s\"", file);
        sendservmsg(msg);

        sendf(-1, 1, "rii", SV_DEMOPLAYBACK, 1);

        if(gzread(demoplayback, &nextplayback, sizeof(nextplayback))!=sizeof(nextplayback))
        {
            enddemoplayback();
            return;
        }
        endianswap(&nextplayback, sizeof(nextplayback), 1);
    }

    void enddemoplayback()
    {
        if(!demoplayback) return;
        gzclose(demoplayback);
        demoplayback = NULL;

        sendf(-1, 1, "rii", SV_DEMOPLAYBACK, 0);

        sendservmsg("demo playback finished");

        loopv(clients)
        {
            ENetPacket *packet = enet_packet_create(NULL, MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            ucharbuf p(packet->data, packet->dataLength);
            welcomepacket(p, clients[i]->clientnum, packet);
            enet_packet_resize(packet, p.length());
            sendpacket(clients[i]->clientnum, 1, packet);
            if(!packet->referenceCount) enet_packet_destroy(packet);
        }
    }

    void readdemo()
    {
        if(!demoplayback) return;
        while(gamemillis>=nextplayback)
        {
            int chan, len;
            if(gzread(demoplayback, &chan, sizeof(chan))!=sizeof(chan) ||
               gzread(demoplayback, &len, sizeof(len))!=sizeof(len))
            {
                enddemoplayback();
                return;
            }
            endianswap(&chan, sizeof(chan), 1);
            endianswap(&len, sizeof(len), 1);
            ENetPacket *packet = enet_packet_create(NULL, len, 0);
            if(!packet || gzread(demoplayback, packet->data, len)!=len)
            {
                if(packet) enet_packet_destroy(packet);
                enddemoplayback();
                return;
            }
            sendpacket(-1, chan, packet);
            if(!packet->referenceCount) enet_packet_destroy(packet);
            if(gzread(demoplayback, &nextplayback, sizeof(nextplayback))!=sizeof(nextplayback))
            {
                enddemoplayback();
                return;
            }
            endianswap(&nextplayback, sizeof(nextplayback), 1);
        }
    }
 
    void changemap(const char *s, int mode,int gametime=600000)
    {
        if(m_demo) enddemoplayback();
        else enddemorecord();
        
        gamecount++;
        mapreload = false;
        gamemode = mode; s_strcpy(sgamemode,modestr(gamemode));
        gamemillis = 0;
        //minremain = m_teammode ? 15 : 10;
        //gamelimit = minremain*60000;
        gamelimit=gametime;
        minremain=gamelimit/60000;
        interm = 0;
        s_strcpy(smapname, s);
        resetitems();
        notgotitems = true;
        scores.setsize(0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        }
        if(m_teammode) autoteam();

        if(m_arena) smode = &arenamode;
        else if(m_capture) smode = &capturemode;
        else if(m_assassin) smode = &assassinmode;
        else if(m_ctf) smode = &ctfmode;
        else smode = NULL;
        if(smode) smode->reset(false);

        if(gamemode>1 || (gamemode==0 && hasnonlocalclients())) sendf(-1, 1, "ri2", SV_TIMEUP, minremain);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->mapchange();
            ci->state.lasttimeplayed = lastmillis;
            if(m_mp(gamemode) && ci->state.state!=CS_SPECTATOR) sendspawn(ci);
        }

        if(m_demo) setupdemoplayback();
        else if(demonextmatch)
        {
            demonextmatch = false;
            setupdemorecord();
        }
        
        clear_pgvars();
        
        var_gametime.readonly(true);
        scriptable_events.dispatch(&on_mapchange,cubescript::args0(),NULL);
        var_gametime.readonly(false);
    }

    savedscore &findscore(clientinfo *ci, bool insert)
    {
        uint ip = getclientip(ci->clientnum);
        if(!ip) return *(savedscore *)0;
        if(!insert) 
        {
            loopv(clients)
            {
                clientinfo *oi = clients[i];
                if(oi->clientnum != ci->clientnum && getclientip(oi->clientnum) == ip && !strcmp(oi->name, ci->name))
                {
                    oi->state.timeplayed += lastmillis - oi->state.lasttimeplayed;
                    oi->state.lasttimeplayed = lastmillis;
                    static savedscore curscore;
                    curscore.save(oi->state);
                    return curscore;
                }
            }
        }
        loopv(scores)
        {
            savedscore &sc = scores[i];
            if(sc.ip == ip && !strcmp(sc.name, ci->name)) return sc;
        }
        if(!insert) return *(savedscore *)0;
        savedscore &sc = scores.add();
        sc.ip = ip;
        s_strcpy(sc.name, ci->name);
        return sc;
    }

    void savescore(clientinfo *ci)
    {
        savedscore &sc = findscore(ci, true);
        if(&sc) sc.save(ci->state);
    }

    struct votecount
    {
        char *map;
        int mode, count;
        votecount() {}
        votecount(char *s, int n) : map(s), mode(n), count(0) {}
    };

    void checkvotes(bool force = false)
    {
        vector<votecount> votes;
        int maxvotes = 0;
        loopv(clients)
        {
            clientinfo *oi = clients[i];
            if(oi->state.state==CS_SPECTATOR && !oi->privilege) continue;
            maxvotes++;
            if(!oi->mapvote[0]) continue;
            votecount *vc = NULL;
            loopvj(votes) if(!strcmp(oi->mapvote, votes[j].map) && oi->modevote==votes[j].mode)
            { 
                vc = &votes[j];
                break;
            }
            if(!vc) vc = &votes.add(votecount(oi->mapvote, oi->modevote));
            vc->count++;
        }
        votecount *best = NULL;
        loopv(votes) if(!best || votes[i].count > best->count || (votes[i].count == best->count && rnd(2))) best = &votes[i];
        if(force || (best && best->count > maxvotes/2))
        {
            if(demorecord) enddemorecord();
            if(best && (best->count > (force ? 1 : maxvotes/2)))
            {
                sendservmsg(force ? "vote passed by default" : "vote passed by majority");
                sendf(-1, 1, "risii", SV_MAPCHANGE, best->map, best->mode, 1);
                
                gamemode=best->mode;
                changemap(best->map, best->mode, m_teammode ? 900000 : 600000);
            }
            else
            {
                gamelimit=600000;
                smapname[0]='\0';
                var_mapname.readonly(false);
                var_gamemode.readonly(false);
                scriptable_events.dispatch(&on_setmap,cubescript::args0(),NULL);
                var_mapname.readonly(true);
                var_gamemode.readonly(true);
                sync_game_settings();
                if(smapname[0])
                {
                    sendf(-1, 1, "risii", SV_MAPCHANGE, smapname, gamemode,1);
                    changemap(smapname,gamemode,gamelimit);
                }
                else
                {
                    mapreload = true;
                    if(clients.length()) sendf(-1, 1, "ri", SV_MAPRELOAD);
                }
            }
        }
    }

    int nonspectators(int exclude = -1)
    {
        int n = 0;
        loopv(clients) if(i!=exclude && clients[i]->state.state!=CS_SPECTATOR) n++;
        return n;
    }

    int checktype(int type, clientinfo *ci)
    {
        if(ci && ci->local) return type;
#if 0
        // other message types can get sent by accident if a master forces spectator on someone, so disabling this case for now and checking for spectator state in message handlers
        // spectators can only connect and talk
        static int spectypes[] = { SV_INITC2S, SV_POS, SV_TEXT, SV_PING, SV_CLIENTPING, SV_GETMAP, SV_SETMASTER };
        if(ci && ci->state.state==CS_SPECTATOR && !ci->privilege)
        {
            loopi(sizeof(spectypes)/sizeof(int)) if(type == spectypes[i]) return type;
            return -1;
        }
#endif
        // only allow edit messages in coop-edit mode
        if(type>=SV_EDITENT && type<=SV_GETMAP && gamemode!=1) return -1;
        // server only messages
        static int servtypes[] = { SV_INITS2C, SV_MAPRELOAD, SV_SERVMSG, SV_DAMAGE, SV_HITPUSH, SV_SHOTFX, SV_DIED, SV_SPAWNSTATE, SV_FORCEDEATH, SV_ARENAWIN, SV_ITEMACC, SV_ITEMSPAWN, SV_TIMEUP, SV_CDIS, SV_CURRENTMASTER, SV_PONG, SV_RESUME, SV_TEAMSCORE, SV_BASEINFO, SV_BASEREGEN, SV_ANNOUNCE, SV_CLEARTARGETS, SV_CLEARHUNTERS, SV_ADDTARGET, SV_REMOVETARGET, SV_ADDHUNTER, SV_REMOVEHUNTER, SV_SENDDEMOLIST, SV_SENDDEMO, SV_DEMOPLAYBACK, SV_SENDMAP, SV_DROPFLAG, SV_SCOREFLAG, SV_RETURNFLAG, SV_CLIENT };
        if(ci) loopi(sizeof(servtypes)/sizeof(int)) if(type == servtypes[i]) return -1;
        return type;
    }

    static void freecallback(ENetPacket *packet)
    {
        extern igameserver *sv;
        ((fpsserver *)sv)->cleanworldstate(packet);
    }

    void cleanworldstate(ENetPacket *packet)
    {
        loopv(worldstates)
        {
            worldstate *ws = worldstates[i];
            if(packet->data >= ws->positions.getbuf() && packet->data <= &ws->positions.last()) ws->uses--;
            else if(packet->data >= ws->messages.getbuf() && packet->data <= &ws->messages.last()) ws->uses--;
            else continue;
            if(!ws->uses)
            {
                delete ws;
                worldstates.remove(i);
            }
            break;
        }
    }

    bool buildworldstate()
    {
        static struct { int posoff, msgoff, msglen; } pkt[MAXCLIENTS];
        worldstate &ws = *new worldstate;
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.position.empty()) pkt[i].posoff = -1;
            else
            {
                pkt[i].posoff = ws.positions.length();
                loopvj(ci.position) ws.positions.add(ci.position[j]);
            }
            if(ci.messages.empty()) pkt[i].msgoff = -1;
            else
            {
                pkt[i].msgoff = ws.messages.length();
                ucharbuf p = ws.messages.reserve(16);
                putint(p, SV_CLIENT);
                putint(p, ci.clientnum);
                putuint(p, ci.messages.length());
                ws.messages.addbuf(p);
                loopvj(ci.messages) ws.messages.add(ci.messages[j]);
                pkt[i].msglen = ws.messages.length() - pkt[i].msgoff;
            }
        }
        int psize = ws.positions.length(), msize = ws.messages.length();
        if(psize) recordpacket(0, ws.positions.getbuf(), psize);
        if(msize) recordpacket(1, ws.messages.getbuf(), msize);
        loopi(psize) { uchar c = ws.positions[i]; ws.positions.add(c); }
        loopi(msize) { uchar c = ws.messages[i]; ws.messages.add(c); }
        ws.uses = 0;
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            ENetPacket *packet;
            if(psize && (pkt[i].posoff<0 || psize-ci.position.length()>0))
            {
                packet = enet_packet_create(&ws.positions[pkt[i].posoff<0 ? 0 : pkt[i].posoff+ci.position.length()], 
                                            pkt[i].posoff<0 ? psize : psize-ci.position.length(), 
                                            ENET_PACKET_FLAG_NO_ALLOCATE);
                sendpacket(ci.clientnum, 0, packet);
                if(!packet->referenceCount) enet_packet_destroy(packet);
                else { ++ws.uses; packet->freeCallback = freecallback; }
            }
            ci.position.setsizenodelete(0);

            if(msize && (pkt[i].msgoff<0 || msize-pkt[i].msglen>0))
            {
                packet = enet_packet_create(&ws.messages[pkt[i].msgoff<0 ? 0 : pkt[i].msgoff+pkt[i].msglen], 
                                            pkt[i].msgoff<0 ? msize : msize-pkt[i].msglen, 
                                            (reliablemessages ? ENET_PACKET_FLAG_RELIABLE : 0) | ENET_PACKET_FLAG_NO_ALLOCATE);
                sendpacket(ci.clientnum, 1, packet);
                if(!packet->referenceCount) enet_packet_destroy(packet);
                else { ++ws.uses; packet->freeCallback = freecallback; }
            }
            ci.messages.setsizenodelete(0);
        }
        reliablemessages = false;
        if(!ws.uses) 
        {
            delete &ws;
            return false;
        }
        else 
        {
            worldstates.add(&ws); 
            return true;
        }
    }

    bool sendpackets()
    {
        if(clients.empty()) return false;
        enet_uint32 curtime = enet_time_get()-lastsend;
        if(curtime<33) return false;
        bool flush = buildworldstate();
        lastsend += curtime - (curtime%33);
        return flush;
    }

    void parsepacket(int sender, int chan, bool reliable, ucharbuf &p)     // has to parse exactly each byte of the packet
    {
        if(sender<0) return;
        if(chan==2)
        {
            receivefile(sender, p.buf, p.maxlen);
            return;
        }
        if(reliable) reliablemessages = true;
        char text[MAXTRANS];
        int cn = -1, type;
        clientinfo *ci = sender>=0 ? (clientinfo *)getinfo(sender) : NULL;
        #define QUEUE_MSG { if(!ci->local) while(curmsg<p.length()) ci->messages.add(p.buf[curmsg++]); }
        #define QUEUE_BUF(size, body) { \
            if(!ci->local) \
            { \
                curmsg = p.length(); \
                ucharbuf buf = ci->messages.reserve(size); \
                { body; } \
                ci->messages.addbuf(buf); \
            } \
        }
        #define QUEUE_INT(n) QUEUE_BUF(5, putint(buf, n))
        #define QUEUE_UINT(n) QUEUE_BUF(4, putuint(buf, n))
        #define QUEUE_STR(text) QUEUE_BUF(2*strlen(text)+1, sendstring(text, buf))
        int curmsg;
        while((curmsg = p.length()) < p.maxlen) switch(type = checktype(getint(p), ci))
        {
            case SV_POS:
            {
                cn = getint(p);
                if(cn<0 || cn>=getnumclients() || cn!=sender)
                {
                    disconnect_client(sender, DISC_CN);
                    return;
                }
                vec oldpos(ci->state.o);
                loopi(3) ci->state.o[i] = getuint(p)/DMF;
                getuint(p);
                loopi(5) getint(p);
                int physstate = getuint(p);
                if(physstate&0x20) loopi(2) getint(p);
                if(physstate&0x10) getint(p);
                getuint(p);
                if(!ci->local && (ci->state.state==CS_ALIVE || ci->state.state==CS_EDITING))
                {
                    ci->position.setsizenodelete(0);
                    while(curmsg<p.length()) ci->position.add(p.buf[curmsg++]);
                }
                if(smode && ci->state.state==CS_ALIVE) smode->moved(ci, oldpos, ci->state.o);
                if(ci->lastupdate==-1) ci->lastupdate=totalmillis;
                else
                {
                    ci->lag=(ci->lag/2)+(max(0,30-(totalmillis-ci->lastupdate))/2);
                    ci->lastupdate=totalmillis;
                }
                break;
            }

            case SV_EDITMODE:
            {
                int val = getint(p);
                if(!ci->local && gamemode!=1) break;
                if(val ? ci->state.state!=CS_ALIVE && ci->state.state!=CS_DEAD : ci->state.state!=CS_EDITING) break;
                if(smode)
                {
                    if(val) smode->leavegame(ci);
                    else smode->entergame(ci);
                }
                if(val)
                {
                    ci->state.editstate = ci->state.state;
                    ci->state.state = CS_EDITING;
                }
                else ci->state.state = ci->state.editstate;
                if(val)
                {
                    ci->events.setsizenodelete(0);
                    ci->state.rockets.reset();
                    ci->state.grenades.reset();
                }
                QUEUE_MSG;
                break;
            }

            case SV_TRYSPAWN:
                if(ci->state.state!=CS_DEAD || ci->state.lastspawn>=0 || (smode && !smode->canspawn(ci))) break;
                if(ci->state.lastdeath) ci->state.respawn();
                sendspawn(ci);
                break;

            case SV_GUNSELECT:
            {
                int gunselect = getint(p);
                if(ci->state.state!=CS_ALIVE) break;
                ci->state.gunselect = gunselect;
                QUEUE_MSG;
                break;
            }

            case SV_SPAWN:
            {
                int ls = getint(p), gunselect = getint(p);
                if((ci->state.state!=CS_ALIVE && ci->state.state!=CS_DEAD) || ls!=ci->state.lifesequence || ci->state.lastspawn<0) break; 
                ci->state.lastspawn = -1;
                ci->state.state = CS_ALIVE;
                ci->state.gunselect = gunselect;
                if(smode) smode->spawned(ci);
                QUEUE_BUF(100,
                {
                    putint(buf, SV_SPAWN);
                    sendstate(ci->state, buf);
                });
                break;
            }

            case SV_SUICIDE:
            {
                gameevent &suicide = ci->addevent();
                suicide.type = GE_SUICIDE;
                break;
            }

            case SV_SHOOT:
            {
                gameevent &shot = ci->addevent();
                shot.type = GE_SHOT;
                #define seteventmillis(event) \
                { \
                    event.id = getint(p); \
                    if(!ci->timesync || (ci->events.length()==1 && ci->state.waitexpired(gamemillis))) \
                    { \
                        ci->timesync = true; \
                        ci->gameoffset = gamemillis - event.id; \
                        event.millis = gamemillis; \
                    } \
                    else event.millis = ci->gameoffset + event.id; \
                }
                seteventmillis(shot.shot);
                shot.shot.gun = getint(p);
                loopk(3) shot.shot.from[k] = getint(p)/DMF;
                loopk(3) shot.shot.to[k] = getint(p)/DMF;
                int hits = getint(p);
                loopk(hits)
                {
                    gameevent &hit = ci->addevent();
                    hit.type = GE_HIT;
                    hit.hit.target = getint(p);
                    hit.hit.lifesequence = getint(p);
                    hit.hit.rays = getint(p);
                    loopk(3) hit.hit.dir[k] = getint(p)/DNF;
                }
                break;
            }

            case SV_EXPLODE:
            {
                gameevent &exp = ci->addevent();
                exp.type = GE_EXPLODE;
                seteventmillis(exp.explode);
                exp.explode.gun = getint(p);
                exp.explode.id = getint(p);
                int hits = getint(p);
                loopk(hits)
                {
                    gameevent &hit = ci->addevent();
                    hit.type = GE_HIT;
                    hit.hit.target = getint(p);
                    hit.hit.lifesequence = getint(p);
                    hit.hit.dist = getint(p)/DMF;
                    loopk(3) hit.hit.dir[k] = getint(p)/DNF;
                }
                break;
            }

            case SV_ITEMPICKUP:
            {
                int n = getint(p);
                gameevent &pickup = ci->addevent();
                pickup.type = GE_PICKUP;
                pickup.pickup.ent = n;
                break;
            }

            case SV_TEXT:
            {
                bool allow=!ci->check_svtext_flooding(svtext_min_interval);
                
                getstring(text, p);
                filtertext(text, text);
                
                if(!allow) break;
                
                bool block=false;
                
                cubescript::arguments a;
                scriptable_events.dispatch(&on_text,a & ci->clientnum & std::string(text),&block);
                
                if(!block)
                {
                    QUEUE_INT(SV_TEXT);
                    QUEUE_STR(text);
                }
                
                break;
            }
            
            case SV_SAYTEAM:
            {
                getstring(text, p);
                
                bool allow=!ci->check_svtext_flooding(svtext_min_interval);
                
                if(ci->state.state==CS_SPECTATOR || !m_teammode || !ci->team[0] || !allow) break;
                loopv(clients)
                {
                    clientinfo *t = clients[i];
                    if(t==ci || t->state.state==CS_SPECTATOR || strcmp(ci->team, t->team)) continue;
                    sendf(t->clientnum, 1, "riis", SV_SAYTEAM, ci->clientnum, text);
                }
                break;
            }

            case SV_INITC2S:
            {
                QUEUE_MSG;
                string oldname; oldname[0]='\0'; if(ci->name[0]) s_strcpy(oldname,ci->name);
                string oldteam; oldteam[0]='\0'; if(ci->team[0]) s_strcpy(oldteam,ci->team);
                bool connected = !ci->name[0];
                getstring(text, p);
                filtertext(text, text, false, MAXNAMELEN);
                if(!text[0]) s_strcpy(text, "unnamed");
                QUEUE_STR(text);
                s_strncpy(ci->name, text, MAXNAMELEN+1);
                if(!ci->local && connected)
                {
                    savedscore &sc = findscore(ci, false);
                    if(&sc) 
                    {
                        sc.restore(ci->state);
                        gamestate &gs = ci->state;
                        sendf(-1, 1, "ri2i9vi", SV_RESUME, sender,
                            gs.state, gs.frags, gs.quadmillis, 
                            gs.lifesequence,
                            gs.health, gs.maxhealth,
                            gs.armour, gs.armourtype,
                            gs.gunselect, GUN_PISTOL-GUN_SG+1, &gs.ammo[GUN_SG], -1);
                    }
                }
                getstring(text, p);
                filtertext(text, text, false, MAXTEAMLEN);
                if(!ci->local && (smode && !smode->canchangeteam(ci, ci->team, text)) && m_teammode)
                {
                    const char *worst = chooseworstteam(text, ci);
                    if(worst)
                    {
                        s_strcpy(text, worst);
                        sendf(sender, 1, "riis", SV_SETTEAM, sender, worst);
                        QUEUE_STR(worst);
                    }
                    else QUEUE_STR(text);
                }
                else QUEUE_STR(text);
                if(smode && ci->state.state==CS_ALIVE && strcmp(ci->team, text)) smode->changeteam(ci, ci->team, text);
                s_strncpy(ci->team, text, MAXTEAMLEN+1);
                QUEUE_MSG;
                
                if(connected)
                {
                    playercount++;
                    ci->connected=true;
                    cubescript::arguments args;
                    scriptable_events.dispatch(&on_connect,args & ci->clientnum,NULL);
                }
                
                if(oldname[0] && strcmp(oldname,ci->name))
                {
                    cubescript::arguments args;
                    scriptable_events.dispatch(&on_rename,args & ci->clientnum & std::string(oldname) & std::string(ci->name),NULL);
                }
                
                if(oldteam[0] && strcmp(oldteam,ci->team))
                {
                    //TODO bool revert=false;
                    cubescript::arguments args;
                    scriptable_events.dispatch(&on_reteam,args & ci->clientnum & std::string(oldteam) & std::string(ci->team),NULL);
                }
                
                break;
            }

            case SV_MAPVOTE:
            {
                getstring(text,p);
                filtertext(text,text);
                int reqmode = getint(p);
                if(!ci->local && !m_mp(reqmode)) reqmode = 0;
                bool allow=!ci->check_flooding(ci->svmapvote_interval,svmapvote_min_interval,"map voting");
                
                cubescript::arguments args;
                bool event_block=false;
                scriptable_events.dispatch(&on_mapvote,
                    args & ci->clientnum & std::string(modestr(reqmode)) & std::string(text),
                    &event_block);
                
                if(allow && !event_block) vote(text,reqmode,sender);
                break;
            }
            
            case SV_MAPCHANGE:
            {
                getstring(text, p);
                filtertext(text, text);
                int reqmode = getint(p);
                if(!mapreload) break;
                if(!ci->local && !m_mp(reqmode)) reqmode = 0;
                vote(text, reqmode, sender);
                break;
            }

            case SV_ITEMLIST:
            {
                if((ci->state.state==CS_SPECTATOR && !ci->privilege) || !notgotitems) { while(getint(p)>=0 && !p.overread()) getint(p); break; }
                int n;
                while((n = getint(p))>=0 && !p.overread())
                {
                    server_entity se = { getint(p), false, 0 };
                    while(sents.length()<=n) sents.add(se);
                    if(gamemode>=0 && (sents[n].type==I_QUAD || sents[n].type==I_BOOST)) sents[n].spawntime = spawntime(sents[n].type);
                    else sents[n].spawned = true;
                }
                notgotitems = false;
                break;
            }

            case SV_TEAMSCORE:
                getstring(text, p);
                getint(p);
                QUEUE_MSG;
                break;

            case SV_BASEINFO:
                getint(p);
                getstring(text, p);
                getstring(text, p);
                getint(p);
                QUEUE_MSG;
                break;

            case SV_BASES:
                if((ci->state.state!=CS_SPECTATOR || ci->privilege) && smode==&capturemode) capturemode.parsebases(p);
                break;

            case SV_REPAMMO:
                if(ci->state.state!=CS_SPECTATOR && smode==&capturemode) capturemode.replenishammo(ci);
                break;

            case SV_TAKEFLAG:
            {
                int flag = getint(p);
                if(ci->state.state!=CS_SPECTATOR && smode==&ctfmode) ctfmode.takeflag(ci, flag); 
                break;
            }

            case SV_INITFLAGS:
                if((ci->state.state!=CS_SPECTATOR || ci->privilege) && smode==&ctfmode) ctfmode.parseflags(p);
                break;

            case SV_PING:
                sendf(sender, 1, "i2", SV_PONG, getint(p));
                break;

            case SV_CLIENTPING:
                ci->ping=getint(p);
                QUEUE_MSG;
                break;

            case SV_MASTERMODE:
            {
                int mm = getint(p);
                if(ci->privilege && mm>=MM_OPEN && mm<=MM_PRIVATE)
                {
                    if(ci->privilege>=PRIV_ADMIN || (mastermask&(1<<mm)))
                    {
                        bool block=false;
                        cubescript::arguments args;
                        scriptable_events.dispatch(&on_chmm,args & ci->clientnum & mm,&block);
                        if(block) break;
                        
                        mastermode = mm;
                        allowedips.setsize(0);
                        if(mm>=MM_PRIVATE) 
                        {
                            loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
                        }
                        s_sprintfd(s)("mastermode is now %s (%d)", mastermodestr(mastermode), mastermode);
                        sendservmsg(s);
                    }
                    else
                    {
                        s_sprintfd(s)("mastermode %d is disabled on this server", mm);
                        sendf(sender, 1, "ris", SV_SERVMSG, s); 
                    }
                }   
                break;
            }
           
            case SV_CLEARBANS:
            {
                if(ci->privilege)
                {
                    bannedips.setsize(0);
                    sendservmsg("cleared all bans");
                }
                break;
            }

            case SV_KICK:
            {
                int victim = getint(p);
                if(ci->privilege && victim>=0 && victim<getnumclients() && ci->clientnum!=victim && getinfo(victim))
                {
                    bool allow=!ci->check_flooding(ci->svkick_interval,svkick_min_interval,"kicking players");
                    
                    if(allow)
                    {
                        bool blocked=false;
                        cubescript::arguments args;
                        scriptable_events.dispatch(&on_kick,args & victim & ci->clientnum,&blocked);
                        if(!blocked) add_pending_ban(victim,-1);
                    }
                }
                break;
            }

            case SV_SPECTATOR:
            {
                int spectator = getint(p), val = getint(p);
                if(!ci->privilege && (ci->state.state==CS_SPECTATOR || spectator!=sender)) break;
                
                set_spectator(spectator,val);
                break;
            }

            case SV_SETTEAM:
            {
                int who = getint(p);
                getstring(text, p);
                filtertext(text, text, false, MAXTEAMLEN);
                if(!ci->privilege || who<0 || who>=getnumclients()) break;
                clientinfo *wi = (clientinfo *)getinfo(who);
                if(!wi) break;
                
                cubescript::arguments args;
                scriptable_events.dispatch(&on_reteam,args & who & std::string(wi->team) & std::string(text),NULL);                
                
                if(!smode || smode->canchangeteam(wi, wi->team, text))
                {
                    if(smode && wi->state.state==CS_ALIVE && strcmp(wi->team, text)) 
                        smode->changeteam(wi, wi->team, text);
                    s_strncpy(wi->team, text, MAXTEAMLEN+1);
                }
                sendf(sender, 1, "riis", SV_SETTEAM, who, wi->team);
                QUEUE_INT(SV_SETTEAM);
                QUEUE_INT(who);
                QUEUE_STR(wi->team);
                break;
            } 

            case SV_FORCEINTERMISSION:
                if(m_sp) startintermission();
                break;

            case SV_RECORDDEMO:
            {
                int val = getint(p);
                if(ci->privilege<PRIV_ADMIN) break;
                recorddemo(val);
                break;
            }

            case SV_STOPDEMO:
            {
                if(!ci->local && ci->privilege<PRIV_ADMIN) break;
                if(m_demo) enddemoplayback();
                else enddemorecord();
                break;
            }

            case SV_CLEARDEMOS:
            {
                int demo = getint(p);
                if(ci->privilege<PRIV_ADMIN) break;
                cleardemos(demo);
                break;
            }

            case SV_LISTDEMOS:
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                listdemos(sender);
                break;

            case SV_GETDEMO:
            {
                int n = getint(p);
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                senddemo(sender, n);
                break;
            }

            case SV_GETMAP:
                if(mapdata)
                {
                    sendf(sender, 1, "ris", SV_SERVMSG, "server sending map...");
                    sendfile(sender, 2, mapdata, "ri", SV_SENDMAP);
                }
                else sendf(sender, 1, "ris", SV_SERVMSG, "no map to send"); 
                break;

            case SV_NEWMAP:
            {
                int size = getint(p);
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                
                bool block=false;
                cubescript::arguments args;
                scriptable_events.dispatch(&on_newmap,args & ci->clientnum & size,&block);
                if(block) break;
                
                if(size>=0)
                {
                    smapname[0] = '\0';
                    resetitems();
                    notgotitems = false;
                    if(smode) smode->reset(true);
                }
                QUEUE_MSG;
                break;
            }

            case SV_SETMASTER:
            {
                int val = getint(p);
                getstring(text, p);
                
                bool allow=!ci->check_flooding(ci->svsetmaster_interval,svsetmaster_min_interval,"requesting master");
                
                if(allow) setmaster(ci, val!=0, text);
                // don't broadcast the master password
                break;
            }

            case SV_APPROVEMASTER:
            {
                int mn = getint(p);
                if(mastermask&MM_AUTOAPPROVE || ci->state.state==CS_SPECTATOR) break;
                clientinfo *candidate = (clientinfo *)getinfo(mn);
                if(!candidate || !candidate->wantsmaster || mn==sender || getclientip(mn)==getclientip(sender)) break;
                
                bool block=false;
                cubescript::arguments args;
                scriptable_events.dispatch(&on_approvemaster,args & candidate->clientnum & sender,&block);
                if(block) break;
                
                setmaster(candidate, true, "", true);
                break;
            }

            default:
            {
                int size = msgsizelookup(type);
                if(size==-1) { ci->disc_reason=DISC_TAGT; disconnect_client(sender, DISC_TAGT); return; }
                if(size>0) loopi(size-1) getint(p);
                if(ci && ci->state.state!=CS_SPECTATOR) QUEUE_MSG;
                break;
            }
        }
    }

    void sendstate(gamestate &gs, ucharbuf &p)
    {
        putint(p, gs.lifesequence);
        putint(p, gs.health);
        putint(p, gs.maxhealth);
        putint(p, gs.armour);
        putint(p, gs.armourtype);
        putint(p, gs.gunselect);
        loopi(GUN_PISTOL-GUN_SG+1) putint(p, gs.ammo[GUN_SG+i]);
    }

    int welcomepacket(ucharbuf &p, int n, ENetPacket *packet)
    {
        clientinfo *ci = (clientinfo *)getinfo(n);
        int hasmap = (gamemode==1 && clients.length()>1) || (smapname[0] && (minremain>0 || (ci && ci->state.state==CS_SPECTATOR) || nonspectators(n)));
        putint(p, SV_INITS2C);
        putint(p, n);
        putint(p, PROTOCOL_VERSION);
        putint(p, hasmap);
        if(hasmap)
        {
            putint(p, SV_MAPCHANGE);
            sendstring(smapname, p);
            putint(p, gamemode);
            putint(p, notgotitems ? 1 : 0);
            if(!ci || gamemode>1 || (gamemode==0 && hasnonlocalclients()))
            {
                putint(p, SV_TIMEUP);
                putint(p, minremain);
            }
            if(!notgotitems)
            {
                putint(p, SV_ITEMLIST);
                loopv(sents) if(sents[i].spawned)
                {
                    putint(p, i);
                    putint(p, sents[i].type);
                    if(p.remaining() < 256)
                    {
                        enet_packet_resize(packet, packet->dataLength + MAXTRANS);
                        p.buf = packet->data;
                    }
                }
                putint(p, -1);
            }
        }
        if(ci && !ci->local && m_teammode)
        {
            const char *worst = chooseworstteam();
            if(worst)
            {
                putint(p, SV_SETTEAM);
                putint(p, ci->clientnum);
                sendstring(worst, p);
            }
        }
        if(ci && (m_demo || m_mp(gamemode)) && ci->state.state!=CS_SPECTATOR)
        {
            if(smode && !smode->canspawn(ci, true))
            {
                ci->state.state = CS_DEAD;
                putint(p, SV_FORCEDEATH);
                putint(p, n);
                sendf(-1, 1, "ri2x", SV_FORCEDEATH, n, n);
            }
            else
            {
                gamestate &gs = ci->state;
                spawnstate(ci);
                putint(p, SV_SPAWNSTATE);
                sendstate(gs, p);
                gs.lastspawn = gamemillis; 
            }
        }
        if(ci && ci->state.state==CS_SPECTATOR)
        {
            putint(p, SV_SPECTATOR);
            putint(p, n);
            putint(p, 1);
            sendf(-1, 1, "ri3x", SV_SPECTATOR, n, 1, n);   
        }
        if(clients.length()>1)
        {
            putint(p, SV_RESUME);
            loopv(clients)
            {
                clientinfo *oi = clients[i];
                if(oi->clientnum==n) continue;
                if(p.remaining() < 256)
                {
                    enet_packet_resize(packet, packet->dataLength + MAXTRANS);
                    p.buf = packet->data;
                }
                putint(p, oi->clientnum);
                putint(p, oi->state.state);
                putint(p, oi->state.frags);
                putint(p, oi->state.quadmillis);
                sendstate(oi->state, p);
            }
            putint(p, -1);
        }
        if(smode) 
        {
            enet_packet_resize(packet, packet->dataLength + MAXTRANS);
            p.buf = packet->data;
            smode->initclient(ci, p, true);
        }
        return 1;
    }

    void checkintermission()
    {
        if(minremain>0)
        {
            minremain = gamemillis>=gamelimit ? 0 : (gamelimit - gamemillis + 60000 - 1)/60000;
            sendf(-1, 1, "ri2", SV_TIMEUP, minremain);
            if(!minremain && smode) smode->intermission();
            
            cubescript::arguments args;
            scriptable_events.dispatch(&on_timeupdate,args & minremain,NULL);
        }
        
        if(!interm && minremain<=0)
        {
            interm = gamemillis+10000;
        }
    }

    void startintermission() { gamelimit = min(gamelimit, gamemillis); checkintermission(); }

    void clearevent(clientinfo *ci)
    {
        int n = 1;
        while(n<ci->events.length() && ci->events[n].type==GE_HIT) n++;
        ci->events.remove(0, n);
    }

    void spawnstate(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        gs.spawnstate(gamemode);
        gs.lifesequence++;
    }

    void sendspawn(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        spawnstate(ci);
        sendf(ci->clientnum, 1, "ri7v", SV_SPAWNSTATE, gs.lifesequence,
            gs.health, gs.maxhealth,
            gs.armour, gs.armourtype,
            gs.gunselect, GUN_PISTOL-GUN_SG+1, &gs.ammo[GUN_SG]);
        gs.lastspawn = gamemillis;
    }

    void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0))
    {
        gamestate &ts = target->state;
        ts.dodamage(damage);
        actor->state.damage += damage;
        sendf(-1, 1, "ri6", SV_DAMAGE, target->clientnum, actor->clientnum, damage, ts.armour, ts.health); 
        if(target!=actor && !hitpush.iszero()) 
        {
            vec v(hitpush);
            if(!v.iszero()) v.normalize();
            sendf(target->clientnum, 1, "ri6", SV_HITPUSH, gun, damage,
                int(v.x*DNF), int(v.y*DNF), int(v.z*DNF));
        }
        if(ts.health<=0)
        {
            target->state.deaths++;
            if(actor!=target && isteam(actor->team, target->team)) actor->state.teamkills++;
            int fragvalue = smode ? smode->fragvalue(target, actor) : (target==actor || isteam(target->team, actor->team) ? -1 : 1);
            actor->state.frags += fragvalue;
            if(fragvalue>0)
            {
                int friends = 0, enemies = 0; // note: friends also includes the fragger
                if(m_teammode) loopv(clients) if(strcmp(clients[i]->team, actor->team)) enemies++; else friends++;
                else { friends = 1; enemies = clients.length()-1; }
                actor->state.effectiveness += fragvalue*friends/float(max(enemies, 1));
            }
            sendf(-1, 1, "ri4", SV_DIED, target->clientnum, actor->clientnum, actor->state.frags);
            target->position.setsizenodelete(0);
            if(smode) smode->died(target, actor);
            ts.state = CS_DEAD;
            ts.lastdeath = gamemillis;
            
            {
                cubescript::arguments args;
                scriptable_events.dispatch(&on_damage,args & actor->clientnum & target->clientnum & damage & std::string(guns[gun].name),NULL);
            }

            cubescript::arguments args;
            scriptable_events.dispatch(&on_death,args & actor->clientnum & target->clientnum,NULL);
            // don't issue respawn yet until DEATHMILLIS has elapsed
            // ts.respawn();
        }
    }

    void processevent(clientinfo *ci, suicideevent &e)
    {
        gamestate &gs = ci->state;
        if(gs.state!=CS_ALIVE) return;
        ci->state.frags += smode ? smode->fragvalue(ci, ci) : -1;
        ci->state.deaths++;
        sendf(-1, 1, "ri4", SV_DIED, ci->clientnum, ci->clientnum, gs.frags);
        ci->position.setsizenodelete(0);
        if(smode) smode->died(ci, NULL);
        gs.state = CS_DEAD;
        gs.respawn();
        
        cubescript::arguments args;
        scriptable_events.dispatch(&on_death,args & ci->clientnum & ci->clientnum,NULL);
    }

    void processevent(clientinfo *ci, explodeevent &e)
    {
        gamestate &gs = ci->state;
        switch(e.gun)
        {
            case GUN_RL:
                if(!gs.rockets.remove(e.id)) return;
                break;

            case GUN_GL:
                if(!gs.grenades.remove(e.id)) return;
                break;

            default:
                return;
        }
        for(int i = 1; i<ci->events.length() && ci->events[i].type==GE_HIT; i++)
        {
            hitevent &h = ci->events[i].hit;
            clientinfo *target = (clientinfo *)getinfo(h.target);
            if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.dist<0 || h.dist>RL_DAMRAD) continue;

            int j = 1;
            for(j = 1; j<i; j++) if(ci->events[j].hit.target==h.target) break;
            if(j<i) continue;

            int damage = guns[e.gun].damage;
            if(gs.quadmillis) damage *= 4;        
            damage = int(damage*(1-h.dist/RL_DISTSCALE/RL_DAMRAD));
            if(e.gun==GUN_RL && target==ci) damage /= RL_SELFDAMDIV;
            dodamage(target, ci, damage, e.gun, h.dir);
        }
    }
        
    void processevent(clientinfo *ci, shotevent &e)
    {
        gamestate &gs = ci->state;
        int wait = e.millis - gs.lastshot;
        if(!gs.isalive(gamemillis) ||
           wait<gs.gunwait ||
           e.gun<GUN_FIST || e.gun>GUN_PISTOL ||
           gs.ammo[e.gun]<=0)
            return;
        if(e.gun!=GUN_FIST) gs.ammo[e.gun]--;
        gs.lastshot = e.millis; 
        gs.gunwait = guns[e.gun].attackdelay; 
        sendf(-1, 1, "ri9x", SV_SHOTFX, ci->clientnum, e.gun,
                int(e.from[0]*DMF), int(e.from[1]*DMF), int(e.from[2]*DMF),
                int(e.to[0]*DMF), int(e.to[1]*DMF), int(e.to[2]*DMF),
                ci->clientnum);
        gs.shotdamage += guns[e.gun].damage*(gs.quadmillis ? 4 : 1)*(e.gun==GUN_SG ? SGRAYS : 1);
        switch(e.gun)
        {
            case GUN_RL: gs.rockets.add(e.id); break;
            case GUN_GL: gs.grenades.add(e.id); break;
            default:
            {
                int totalrays = 0, maxrays = e.gun==GUN_SG ? SGRAYS : 1;
                for(int i = 1; i<ci->events.length() && ci->events[i].type==GE_HIT; i++)
                {
                    hitevent &h = ci->events[i].hit;
                    clientinfo *target = (clientinfo *)getinfo(h.target);
                    if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.rays<1) continue;

                    totalrays += h.rays;
                    if(totalrays>maxrays) continue;
                    int damage = h.rays*guns[e.gun].damage;
                    if(gs.quadmillis) damage *= 4;
                    dodamage(target, ci, damage, e.gun, h.dir);
                }
                break;
            }
        }
    }

    void processevent(clientinfo *ci, pickupevent &e)
    {
        gamestate &gs = ci->state;
        if(m_mp(gamemode) && !gs.isalive(gamemillis)) return;
        pickup(e.ent, ci->clientnum);
    }

    void processevents()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(curtime>0 && ci->state.quadmillis) ci->state.quadmillis = max(ci->state.quadmillis-curtime, 0);
            while(ci->events.length())
            {
                gameevent &e = ci->events[0];
                if(e.type<GE_SUICIDE)
                {
                    if(e.shot.millis>gamemillis) break;
                    if(e.shot.millis<ci->lastevent) { clearevent(ci); continue; }
                    ci->lastevent = e.shot.millis;
                }
                switch(e.type)
                {
                    case GE_SHOT: processevent(ci, e.shot); break;
                    case GE_EXPLODE: processevent(ci, e.explode); break;
                    // untimed events
                    case GE_SUICIDE: processevent(ci, e.suicide); break;
                    case GE_PICKUP: processevent(ci, e.pickup); break;
                }
                clearevent(ci);
            }
        }
    }
                         
    void serverupdate(int _lastmillis, int _totalmillis)
    {
        curtime = _lastmillis - lastmillis;
        gamemillis += curtime;
        lastmillis = _lastmillis;
        totalmillis = _totalmillis;
        
        try
        {
            m_sleep_jobs.run();
            m_script_pipes.run();
        }
        catch(const cubescript::script_error<cubescript::symbol_error> & e)
            {std::cerr<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<": "<<e.get_id()<<std::endl;}
        catch(const cubescript::script_error<cubescript::error_key> & e)
            {std::cerr<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<" in expression \""<<e.get_expr()<<"\""<<std::endl;}
        catch(const cubescript::expr_error<cubescript::error_key> & e)
            {std::cerr<<"cubescript error: "<<e.what()<<" in expression \""<<e.get_expr()<<"\"."<<std::endl;}
        catch(const cubescript::symbol_error & e)
            {std::cerr<<"cubescript error: "<<e.what()<<": "<<e.get_id()<<std::endl;}
        catch(const cubescript::error_key & e)
            {std::cerr<<"cubescript error: "<<e.what()<<std::endl;}
        
        if(pending_bans)
        {
            loopv(clients)
            {
                if(clients[i]->ban)
                {
                    clients[i]->disc_reason=DISC_KICK;
                    
                    int cn=clients[i]->clientnum;
                    
                    ban &b = bannedips.add();
                    b.time = totalmillis;
                    b.expire = clients[i]->bantime!=-1 ? totalmillis+(clients[i]->bantime*60000) : 0;
                    b.ip = getclientip(cn);
                    allowedips.removeobj(b.ip);
                    
                    disconnect_client(cn, DISC_KICK);
                    
                    pending_bans--;
                }
                
                if(!pending_bans) break;
            }
            pending_bans=0;
        }
        
        if(m_demo) readdemo();
        else if(minremain>0)
        {
            processevents();
            if(curtime) 
            {
                loopv(sents) if(sents[i].spawntime) // spawn entities when timer reached
                {
                    int oldtime = sents[i].spawntime;
                    sents[i].spawntime -= curtime;
                    if(sents[i].spawntime<=0)
                    {
                        sents[i].spawntime = 0;
                        sents[i].spawned = true;
                        sendf(-1, 1, "ri2", SV_ITEMSPAWN, i);
                    }
                    else if(sents[i].spawntime<=10000 && oldtime>10000 && (sents[i].type==I_QUAD || sents[i].type==I_BOOST))
                    {
                        sendf(-1, 1, "ri2", SV_ANNOUNCE, sents[i].type);
                    }
                }
            }
            if(smode) smode->update();
        }

        while(bannedips.length() && bannedips[0].time-totalmillis>4*60*60000) bannedips.remove(0);
        
        if(masterupdate) 
        { 
            clientinfo *m = currentmaster>=0 ? (clientinfo *)getinfo(currentmaster) : NULL;
            loopv(clients)
                if(clients[i]->privilege==PRIV_NONE || currentmaster==clients[i]->clientnum)
                    sendf(clients[i]->clientnum, 1, "ri3", SV_CURRENTMASTER, currentmaster, m ? m->privilege : 0);
            masterupdate = false;
        }
        
        if((gamemode>1 || (gamemode==0 && hasnonlocalclients())) && gamemillis-curtime>0 && gamemillis/60000!=(gamemillis-curtime)/60000) checkintermission();
        if(interm && gamemillis>interm)
        {
            if(demorecord) enddemorecord();
            interm = 0;
            checkvotes(true);
        }
    }

    bool serveroption(char *arg)
    {
        if(arg[0]=='-') switch(arg[1])
        {
            case 'n': s_strcpy(serverdesc, &arg[2]); return true;
            case 'p': s_strcpy(masterpass, &arg[2]); return true;
            case 'o': if(atoi(&arg[2])) mastermask = (1<<MM_OPEN) | (1<<MM_VETO); return true;
        }
        return false;
    }

    void serverinit()
    {
        smapname[0] = '\0';
        resetitems();
        
        on_startup.push_handler_code("exec [conf/server.conf]");
        
        var_mapname.readonly(false);
        var_gamemode.readonly(false);
        scriptable_events.dispatch(&on_startup,cubescript::args0(),NULL);
        var_mapname.readonly(true);
        var_gamemode.readonly(true);
        sync_game_settings();
        
        struct sigaction terminate_action;
        sigemptyset(&terminate_action.sa_mask);
        terminate_action.sa_handler=shutdown_from_signal;
        terminate_action.sa_flags=0;
        
        sigaction(SIGINT,&terminate_action,NULL);
        sigaction(SIGTERM,&terminate_action,NULL);
    }
    
    const char *privname(int type)
    {
        switch(type)
        {
            case PRIV_ADMIN: return "admin";
            case PRIV_MASTER: return "master";
            default: return "unknown";
        }
    }

    void setmaster(clientinfo *ci, bool val, const char *pass = "", bool approved = false)
    {
        if(approved && (!val || !ci->wantsmaster)) return;
        const char *name = "";
        if(val)
        {
            if(ci->privilege)
            {
                if(!masterpass[0] || !pass[0]==(ci->privilege!=PRIV_ADMIN)) return;
            }
            else if(ci->state.state==CS_SPECTATOR && (!masterpass[0] || strcmp(masterpass, pass))) return;
            loopv(clients) if(ci!=clients[i] && clients[i]->privilege && !clients[i]->hidden_priv)
            {
                if(masterpass[0] && !strcmp(masterpass, pass)) clients[i]->privilege = PRIV_NONE;
                else return;
            }
            if(masterpass[0] && !strcmp(masterpass, pass))
            {
                ci->privilege = PRIV_ADMIN;
                if(currentmaster>=0) ((clientinfo *)getinfo(currentmaster))->privilege=PRIV_NONE;
            }
            else if(!approved && !(mastermask&MM_AUTOAPPROVE) && !ci->privilege)
            {
                ci->wantsmaster = true;
                s_sprintfd(msg)("%s wants master. Type \"/approve %d\" to approve.", colorname(ci), ci->clientnum);
                sendservmsg(msg);
                return;
            }
            else ci->privilege = PRIV_MASTER;
            name = privname(ci->privilege);
        }
        else
        {
            if(!ci->privilege) return;
            name = privname(ci->privilege);
            ci->privilege = 0;
        }
        
        if(!ci->hidden_priv)
        {
            mastermode = MM_OPEN;
            allowedips.setsize(0);
            s_sprintfd(msg)("%s %s %s", colorname(ci), val ? (approved ? "approved for" : "claimed") : "relinquished", name);
            sendservmsg(msg);
            currentmaster = val ? ci->clientnum : -1;
            masterupdate = true;
            loopv(clients) clients[i]->wantsmaster = false;
        }
        
        cubescript::arguments args;
        scriptable_events.dispatch(&on_setmaster,args & ci->clientnum & val,NULL);
    }

    void localconnect(int n)
    {
        clientinfo *ci = (clientinfo *)getinfo(n);
        ci->clientnum = n;
        ci->local = true;
        clients.add(ci);
    }

    void localdisconnect(int n)
    {
        clientinfo *ci = (clientinfo *)getinfo(n);
        if(smode) smode->leavegame(ci, true);
        clients.removeobj(ci);
    }

    int clientconnect(int n, uint ip)
    {
        clientinfo *ci = (clientinfo *)getinfo(n);
        ci->clientnum = n;
        ci->connect_time=totalmillis;
        ci->connect_id=++concount;
        clients.add(ci);
        loopv(bannedips) if(bannedips[i].ip==ip) return DISC_IPBAN;
        if(mastermode>=MM_PRIVATE) 
        {
            if(allowedips.find(ip)<0) return DISC_PRIVATE;
        }
        if(mastermode>=MM_LOCKED) ci->state.state = CS_SPECTATOR;
        if(currentmaster>=0) masterupdate = true;
        ci->state.lasttimeplayed = lastmillis;
        return DISC_NONE;
    }

    void clientdisconnect(int n) 
    { 
        clientinfo *ci = (clientinfo *)getinfo(n);
        if(ci->privilege) setmaster(ci, false);
        if(smode) smode->leavegame(ci, true);
        ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed; 
        savescore(ci);
        sendf(-1, 1, "ri2", SV_CDIS, n);
        
        if(ci->connected) playercount--;
        
        if(playercount==0)
        {
            var_mapname.readonly(false);
            var_gamemode.readonly(false);
        }
        
        cubescript::arguments args;
        scriptable_events.dispatch(&on_disconnect,args & n & ci->disc_reason,NULL);
        
        clients.removeobj(ci);
        
        if(clients.empty())
        {
            bannedips.setsize(0); // bans clear when server empties
            var_mapname.readonly(true);
            var_gamemode.readonly(true);
            sync_game_settings();
        }
        else checkvotes();
    }

    const char *servername() { return "sauerbratenserver"; }
    int serverinfoport() { return SAUERBRATEN_SERVINFO_PORT; }
    int serverport() { return SAUERBRATEN_SERVER_PORT; }
    const char *getdefaultmaster() { return "sauerbraten.org/masterserver/"; } 

    #include "extinfo.h"

    void serverinforeply(ucharbuf &req, ucharbuf &p)
    {
        if(!getint(req))
        {
            extserverinforeply(req, p);
            return;
        }

        putint(p, clients.length());
        putint(p, 5);                   // number of attrs following
        putint(p, PROTOCOL_VERSION);    // a // generic attributes, passed back below
        putint(p, gamemode);            // b
        putint(p, minremain);           // c
        putint(p, maxclients);
        putint(p, mastermode);
        sendstring(smapname, p);
        sendstring(serverdesc, p);
        sendserverinforeply(p);
    }

    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np)
    {
        return attr.length() && attr[0]==PROTOCOL_VERSION;
    }

    void receivefile(int sender, uchar *data, int len)
    {
        if(gamemode != 1 || len > 1024*1024) return;
        clientinfo *ci = (clientinfo *)getinfo(sender);
        if(ci->state.state==CS_SPECTATOR && !ci->privilege) return;
        if(mapdata) { fclose(mapdata); mapdata = NULL; }
        if(!len) return;
        mapdata = tmpfile();
        if(!mapdata) { sendf(sender, 1, "ris", SV_SERVMSG, "failed to open temporary file for map"); return; }
        fwrite(data, 1, len, mapdata);
        s_sprintfd(msg)("[%s uploaded map to server, \"/getmap\" to receive it]", colorname(ci));
        sendservmsg(msg);
    }
    
    bool duplicatename(clientinfo *ci, char *name)
    {
        if(!name) name = ci->name;
        loopv(clients) if(clients[i]!=ci && !strcmp(name, clients[i]->name)) return true;
        return false;
    }
    
    bool duplicatename(int cn)
    {
        return duplicatename(get_ci(cn),NULL);
    }
    
    char *colorname(clientinfo *ci, char *name = NULL)
    {
        if(!name) name = ci->name;
        if(name[0] && !duplicatename(ci, name)) return name;
        static string cname;
        s_sprintf(cname)("%s \fs\f5(%d)\fr", name, ci->clientnum);
        return cname;
    }
    
    void_ set_flood_protection(const std::string & ptype,int value)
    {
        stopwatch::milliseconds * var=NULL;
        
        if(ptype=="SV_TEXT" || ptype=="SV_SAYTEAM")     var=&svtext_min_interval;
        else if(ptype=="SV_SETMASTER")                  var=&svsetmaster_min_interval;
        else if(ptype=="SV_MAPVOTE")                    var=&svmapvote_min_interval;
        else if(ptype=="SV_KICK")                       var=&svkick_min_interval;
        
        *var=(stopwatch::milliseconds)value;
        
        return void_();
    }
    
    void_ log_status(const std::string & msg)
    {
        std::cout<<msg<<std::endl;
        return void_();
    }
    
    void_ log_error(const std::string & msg)
    {
        std::cerr<<msg<<std::endl;
        return void_();
    }
    
    void_ send_msg(const std::string & msg)
    {
        sendservmsg(msg.c_str());
        return void_();
    }
    
    void_ send_privmsg(int cn,const std::string & msg)
    {
        get_ci(cn)->sendprivmsg(msg.c_str());
        return void_();
    }
    
    std::string get_player_name(int cn)
    {
        return get_ci(cn)->name;
    }
    
    std::string ip_ntoa(uint ip)
    {
        std::ostringstream ipstr;
        ipstr<<((ip>>0) & 0xff)<<"."
             <<((ip>>8) & 0xff)<<"."
             <<((ip>>16) & 0xff)<<"."
             <<((ip>>24) & 0xff);
        return ipstr.str();
    }
    
    std::string get_player_ip(int cn)
    {
        get_ci(cn);
        return ip_ntoa(getclientip(cn));
    }
    
    std::string get_player_team(int cn)
    {
        return get_ci(cn)->team;
    }
    
    void_ kickban(std::list<std::string> & arglist,cubescript::domain *)
    {
        int cn=cubescript::functionN::pop_arg<int>(arglist);
        int mins=-1;
        if(arglist.size()) mins=cubescript::functionN::pop_arg<int>(arglist);
        add_pending_ban(cn,mins);
        return void_();
    }
    
    void add_pending_ban(int cn,int mins)
    {
        clientinfo * ci=get_ci(cn);
        
        if(!ci->ban)
        {
            pending_bans++;
            ci->ban=true;
            ci->bantime=mins;
        }
    }
    
    std::string get_disc_reason(int reason)
    {
        static const char *disc_reasons[] = { "normal", "end of packet", "client num", "kicked and banned", "tag type", "ip is banned", "server is in private mode", "server FULL (maxclients)" };
        return std::string(disc_reasons[reason]);
    }
    
    void_ setpriv(int cn,const std::string & level)
    {
        int privilege;
        
        if(level=="master") privilege=PRIV_MASTER;
        else if(level=="admin") privilege=PRIV_ADMIN;
        else if(level=="none") privilege=PRIV_NONE;
        else return void_();
        
        clientinfo * ci=get_ci(cn);
        int oldpriv=ci->privilege;
        
        if(currentmaster!=-1 && currentmaster==cn && privilege==PRIV_NONE) setmaster(ci,false);
        else if(oldpriv!=PRIV_NONE && privilege==PRIV_NONE)
        {
            std::ostringstream nonprivmsg;
            nonprivmsg<<ConColour_Info<<"Your invisible "<<privname(oldpriv)<<" status has been revoked.";
            ci->sendprivmsg(nonprivmsg.str().c_str());
            sendf(cn, 1, "ri3", SV_CURRENTMASTER, cn, 0);
        }
        
        ci->hidden_priv=true;
        ci->privilege=privilege;
        
        if(oldpriv==PRIV_NONE && ci->privilege!=PRIV_NONE)
        {
            std::ostringstream privmsg;
            privmsg<<ConColour_Info<<"You have been given invisible "<<privname(ci->privilege)<<" status.";
            ci->sendprivmsg(privmsg.str().c_str());
            sendf(cn, 1, "ri3", SV_CURRENTMASTER, cn, ci->privilege);
        }
        
        return void_();
    }
    
    void_ clearbans()
    {
        bannedips.setsize(0);
        sendservmsg("cleared all bans");
        return void_();
    }
    
    void_ set_interm(int milli)
    {
        if(minremain>0)
        {
            minremain=0;
            gamelimit=0;
            sendf(-1, 1, "ri2", SV_TIMEUP, 0);
            if(smode) smode->intermission();
        }
        interm=gamemillis+milli;
        return void_();
    }
    
    void sync_game_settings()
    {
        int newgamemode=modecode(sgamemode);
        if(newgamemode!=-1) gamemode=newgamemode;
        minremain=gamelimit/60000;
    }
    
    void_ set_spectator(int cn,bool val)
    {
        clientinfo *spinfo = (clientinfo *)getinfo(cn);
        if(!spinfo) return void_();
        
        sendf(-1, 1, "ri3", SV_SPECTATOR, cn, val);
        
        if(spinfo->state.state!=CS_SPECTATOR && val)
        {
            if(smode) smode->leavegame(spinfo);
            spinfo->state.state = CS_SPECTATOR;
            spinfo->state.timeplayed += lastmillis - spinfo->state.lasttimeplayed;
        }
        else if(spinfo->state.state==CS_SPECTATOR && !val)
        {
            spinfo->state.state = CS_DEAD;
            spinfo->state.respawn();
            if(!smode || smode->canspawn(spinfo)) sendspawn(spinfo);
            spinfo->state.lasttimeplayed = lastmillis;
        }
        
        cubescript::arguments args;
        scriptable_events.dispatch(&on_spectator,args & cn & val,NULL);
        
        return void_();
    }
    
    std::string get_player_status(int cn)
    {
        std::string status;
        
        switch(get_ci(cn)->state.state)
        {
            case CS_ALIVE: status="alive"; break;
            case CS_DEAD: status="dead"; break;
            case CS_SPAWNING: status="spawning"; break;
            case CS_LAGGED: status="lagged"; break;
            case CS_SPECTATOR: status="spectator"; break;
            case CS_EDITING: status="editing"; break;
        }
        
        return status;
    }
    
    std::vector<int> players()
    {
        std::vector<int> cnv(clients.length());
        loopv(clients) cnv[i]=((clientinfo *)clients[i])->clientnum;
        return cnv;
    }
    
    int get_player_contime(int cn)
    {
        return (totalmillis-get_ci(cn)->connect_time)/1000;
    }
    
    int get_player_conid(int cn)
    {
        return get_ci(cn)->connect_id;
    }
    
    std::string get_player_priv(int cn)
    {
        int priv=get_ci(cn)->privilege;
        std::string privname;
        switch(priv)
        {
            case PRIV_NONE: privname="none"; break;
            case PRIV_MASTER: privname="master"; break;
            case PRIV_ADMIN: privname="admin"; break;
        }
        return privname;
    }
    
    void_ console_setmaster(int cn,bool value)
    {
        if(value) get_ci(cn)->wantsmaster=true;
        setmaster(get_ci(cn),value,"",value ? true : false);
        return void_();
    }
    
    void_ approvemaster(int cn)
    {
        setmaster(get_ci(cn), true, "", true);
        cubescript::arguments args;
        scriptable_events.dispatch(&on_approvemaster,args & cn & -1,NULL);
        return void_();
    }
    
    int get_player_frags(int cn){return get_ci(cn)->state.frags;}
    std::string get_player_gun(int cn){return guns[get_ci(cn)->state.gunselect].name;}
    int get_player_health(int cn){return get_ci(cn)->state.health;}
    int get_player_maxhealth(int cn){return get_ci(cn)->state.maxhealth;}
    
    std::string access_player_var(std::list<std::string> & arglist,cubescript::domain * aDomain,clientinfo::var_type type)
    {
        bool set=false;
        int cn=cubescript::functionN::pop_arg<int>(arglist);
        std::string name=cubescript::functionN::pop_arg<std::string>(arglist);
        std::string value;
        if(!arglist.empty())
        {
            set=true;
            value=cubescript::functionN::pop_arg<std::string>(arglist);
        }
        std::pair<clientinfo::var_type,std::string> & var=vars[get_ci(cn)->id()][name];
        if(set) var=std::pair<clientinfo::var_type,std::string>(type,value);
        return var.second;
    }
    
    bool has_player_var(int cn,const std::string & name)
    {
        player_map<clientinfo::varmap>::iterator playerIt=vars.find(get_ci(cn)->id());
        if(playerIt==vars.end()) return false;
        return playerIt->second.find(name)!=playerIt->second.end();
    }
    
    void_ changemap_(std::string modename,std::string mapname)
    {
        int gmode=modecode(modename.c_str());
        if(gmode==-1) throw cubescript::error_key("runtime.function.changemap.invalid_gamemode");
        sendf(-1, 1, "risii", SV_MAPCHANGE, mapname.c_str(), gmode, 1);
        changemap(mapname.c_str(),gmode);
        return void_();
    }
    
    void_ recorddemo(bool val){recorddemo(val,""); return void_();}
    void_ recorddemo(bool val,const std::string & filename)
    {
        demonextmatch = val!=0;
        demofilename=filename;
        s_sprintfd(msg)("demo recording is %s for next match", demonextmatch ? "enabled" : "disabled"); 
        sendservmsg(msg);
        return void_();
    }
    
    void_ stopdemo()
    {
        if(m_demo) enddemoplayback();
        else enddemorecord();
        return void_();
    }
    
    void_ add_allowhost(const std::string & hostname)
    {
        hostent * result=gethostbyname(hostname.c_str());
        if( result && 
            result->h_addrtype==AF_INET && 
            result->h_length==4 && 
            result->h_addr_list[0] ) allowedips.add(*((in_addr_t *)result->h_addr_list[0]));
        return void_();
    }
    
    void_ add_denyhost(const std::string & hostname)
    {
        hostent * result=gethostbyname(hostname.c_str());
        if( result && 
            result->h_addrtype==AF_INET && 
            result->h_length==4 && 
            result->h_addr_list[0] )
        {
            in_addr_t ip=*((in_addr_t *)result->h_addr_list[0]);
            ban &b = bannedips.add();
            b.time = totalmillis;
            b.ip = ip;
            allowedips.removeobj(b.ip);
        }
        return void_();
    }
    
    int get_player_ping(int cn){return get_ci(cn)->ping;}
    int get_player_lag(int cn){return get_ci(cn)->lag;}
    
    int get_capture_score(const std::string & teamname)
    {
        if(!m_capture) throw cubescript::error_key("runtime.function.capture_score.wrong_gamemode");
        loopv(capturemode.scores) if(teamname==capturemode.scores[i].team) return capturemode.scores[i].total;
        throw cubescript::error_key("runtime.function.capture_score.team_not_found");
    }
    
    void clear_pgvars()
    {
        for(player_map<clientinfo::varmap>::iterator player=vars.begin(); player!=vars.end(); ++player)
            for(clientinfo::varmap::iterator var=player->second.begin(); var!=player->second.end(); ++var)
                if(var->second.first==clientinfo::GAME_VAR) player->second.erase(var);
    }
    
    bool is_shell_syntax(char c)
    {
        switch(c)
        {
            case '\\':case '\"':case '\'':case '$':case '&':case '>':case '<':case '*':
            case '?':case '[':case ']':case '(':case ')':case '|':case '`':case ';':
            case '#':case '^':case '!':case '~':case '\r':case '\n':case ' ':case '\t':
            case '\0': return true;
            default: return false;
        }
        return false;
    }

    std::string shell_quote(const std::string & str)
    {
        std::string result;
        for(unsigned int i=0; i<str.length(); i++)
        {
            if(is_shell_syntax(str[i])) result+='\\';
            result+=str[i];
        }
        return result;
    }
    
    inline bool is_teamgame(){return m_teammode;}
    inline bool is_itemsgame(){return !m_noitems;}
    inline bool is_capturegame(){return m_capture;}
    inline bool is_ctfgame(){return m_ctf;}
    
    std::string nsresolve(const std::string & hostname)
    {
        hostent * result=gethostbyname(hostname.c_str());
        if( result && 
            result->h_addrtype==AF_INET && 
            result->h_length==4 && 
            result->h_addr_list[0] )
          return ip_ntoa(*((in_addr_t *)result->h_addr_list[0]));
        else return "0.0.0.0";
    }
    
    void_ shutdown()
    {
        scriptable_events.dispatch(&on_shutdown,cubescript::args0(),NULL);
        cleanupserver();
        m_script_pipes.shutdown();
        for(std::list<FILE *>::iterator it=m_logfiles.begin(); it!=m_logfiles.end(); ++it) fclose(*it);
        exit(0);
    }
    
    void_ create_restarter()
    {
        pid_t child=fork();
        if(child!=0)
        {
            string stdfile;
            s_strcpy(stdfile,ttyname(1));
            int maxfd=getdtablesize(); for(int i=0; i<maxfd; i++) ::close(i);
            int status;
            waitpid(child,&status,0);
            sleep(1);
            umask(0);
            open("/dev/null",O_RDONLY);
            dup2(open(stdfile,O_CREAT | O_WRONLY | O_APPEND,484),2);
            execv(g_argv[0],g_argv);
        }
        return void_();
    }
    
    void_ write_to_logfile(const std::string & msg,FILE * file)
    {
        fputs(msg.c_str(),file);
        fputs("\n",file);
        fflush(file);
        return void_();
    }
    
    void_ create_logfile_function(std::string filename,std::string funcname)
    {
        FILE * file=fopen(filename.c_str(),"a"); //fclose called in fpsserver::shutdown()
        if(!file) switch(errno)
        {
            case EACCES: throw cubescript::error_key("runtime.function.logfile.write_permission_denied");
            default: throw cubescript::error_key("runtime.function.logfile.fopen_failed");
        }
        
        m_logfiles.push_back(file);
        
        if(server_domain.lookup_symbol(funcname)) throw cubescript::error_key("runtime.function.logfile.function_name_in_use");
        
        cubescript::symbol * newfunc=new cubescript::function1<void_,const std::string &>(boost::bind(&fpsserver::write_to_logfile,this,_1,file));
        server_domain.register_symbol(funcname,newfunc,cubescript::domain::ADOPT_SYMBOL);
        
        return void_();
    }
    
    inline void log_daemon_error(const std::string & msg)
    {
        FILE * file=fopen("logs/daemon.log","a");
        if(!file) return;
        fputs(msg.c_str(),file);
        fputs("\n",file);
        fclose(file);
    }
    
    pid_t spawn_daemon(const std::string & filename,const std::vector<std::string> & args,const std::string & stdoutfile,const std::string & stderrfile)
    {
        pid_t pid=fork();
        
        if(pid==0)
        {
            int maxfd=getdtablesize();
            for(int i=0; i<maxfd; i++) ::close(i);
            umask(0);
            setsid();
            
            if(open("/dev/null",O_RDONLY)==-1)
            {
                log_daemon_error("cannot open /dev/null");
                exit(1);
            }
            
            if(open(stdoutfile.c_str(),O_WRONLY | O_APPEND | O_CREAT,420)==-1)
            {
                log_daemon_error("cannot open stdout file");
                exit(1);
            }
            
            if(open(stderrfile.c_str(),O_WRONLY | O_APPEND | O_CREAT,420)==-1)
            {
                log_daemon_error("cannot open stderr file");
                exit(1);
            }
            
            char ** argv=new char * [args.size()+1];
            for(unsigned int i=0; i<args.size(); i++) argv[i]=newstring(args[i].c_str());
            argv[args.size()]=NULL;
            
            execv(filename.c_str(),argv);
            
            std::ostringstream execfail;
            execfail<<filename<<" not executed."<<std::endl;
            log_daemon_error(execfail.str());
            
            exit(1);
        }
        else if(pid==-1) throw cubescript::error_key("runtime.function.daemon.fork_failed");
        
        return pid;
    }
    
    void_ kill_process(pid_t pid)
    {
        if(::kill(pid,SIGTERM)==-1)
        {
            switch(errno)
            {
                case EPERM: throw cubescript::error_key("runtime.function.kill.permission_denied");
                case ESRCH: throw cubescript::error_key("runtime.function.kill.pid_not_found");
                default: throw cubescript::error_key("runtime.function.kill.kill_failed");
            }
        }
        return void_();
    }
    
    void_ server_sleep(int ms)
    {
        timespec sleeptime;
        sleeptime.tv_sec=ms/1000;
        sleeptime.tv_nsec=(ms-sleeptime.tv_sec*1000)*1000000;
        if(nanosleep(&sleeptime,&sleeptime)==-1) throw cubescript::error_key("runtime.function.server_sleep.returned_early");
        return void_();
    }
};

static void shutdown_from_signal(int i)
{
    fpsserver * svobj=dynamic_cast<fpsserver *>(sv);
    if(svobj) svobj->shutdown();
    exit(1);
}
