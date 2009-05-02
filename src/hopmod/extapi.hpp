#ifndef HOPMOD_EXTAPI_HPP
#define HOPMOD_EXTAPI_HPP

#include "lua.hpp"
#include <string>
#include <vector>

namespace server
{
    namespace aiman
    {
        extern int botlimit;
        extern bool botbalance;
    }
    
    extern string serverdesc;
    extern string smapname;
    extern string serverpass;
    extern string masterpass;
    extern int currentmaster;
    extern int minremain;
    extern bool reassignteams;
    
    extern int mastermode;
    extern string next_gamemode;
    extern string next_mapname;
    extern int next_gametime;
    
    extern bool allow_mm_veto;
    extern bool allow_mm_locked;
    extern bool allow_mm_private;
    extern bool allow_item[11];
    
    extern void init_hopmod();
    extern void reload_hopmod();
    extern void update_hopmod();
    
    extern int player_sessionid(int);
    extern int player_id(int);
    extern void player_msg(int,const char *);
    extern const char * player_name(int);
    extern const char * player_team(int);
    extern const char * player_privilege(int);
    extern int player_privilege_code(int);
    extern int player_ping(int);
    extern int player_lag(int);
    extern const char * player_ip(int);
    extern int player_iplong(int);
    extern const char * player_status(int);
    extern int player_status_code(int);
    extern int player_frags(int);
    extern int player_deaths(int);
    extern int player_suicides(int);
    extern int player_teamkills(int);
    extern int player_damage(int);
    extern int player_damagewasted(int);
    extern int player_maxhealth(int);
    extern int player_health(int);
    extern int player_gun(int);
    extern int player_hits(int);
    extern int player_shots(int);
    extern int player_accuracy(int);
    extern int player_connection_time(int);
    extern int player_timeplayed(int);
    extern int player_win(int);
    extern void player_spec(int);
    extern void player_unspec(int);
    extern int player_bots(int);
    extern void unsetmaster();
    extern void server_setmaster(int);
    extern void server_setadmin(int);
    extern void player_slay(int);
    extern bool player_changeteam(int,const char *);
    extern int player_authreq(int);
    extern int player_rank(int);
    extern bool player_isbot(int);
    
    extern void team_msg(const char *,const char *);
    extern std::vector<std::string> get_teams();
    extern int lua_team_list(lua_State * L);
    extern int get_team_score(const char * );
    extern std::vector<int> get_team_players(const char * team);
    extern int lua_team_players(lua_State *);
    extern int team_win(const char *);
    extern int team_draw(const char *);
    
    extern void pausegame(bool);
    extern void kick(int cn,int time,const std::string & admin,const std::string & reason);
    extern void changetime(int remaining);
    extern void clearbans();
    extern void changemap(const char * map,const char * mode,int mins);
    extern int modecode(const char * modename);
    extern int getplayercount();
    extern int getbotcount();
    extern int getspeccount();
    extern bool writebanlist(const char * filename);
    extern bool loadbanlist(const char * filename);
    extern void addpermban(const char * addr);
    extern void unsetban(const char * addr);
    extern void delbot(int);
    extern void enable_master_auth(bool);
    extern void update_mastermask();
    extern const char * gamemodename();
    extern int lua_gamemodeinfo(lua_State *);
    extern int recorddemo();
    extern void enddemorecord();
    extern void calc_player_ranks(const char * team);
    
    extern std::vector<int> cs_player_list();
    extern std::vector<int> cs_spec_list();
    extern int lua_player_list(lua_State *);
    extern int lua_spec_list(lua_State *);

    extern bool selectnextgame();
    
    extern int lua_genkeypair(lua_State *);
    extern int lua_genchallenge(lua_State *);
    extern int lua_checkchallenge(lua_State *);
    extern int lua_freechalanswer(lua_State *);
    
    extern void delegateauth(int);
    extern void relayauthanswer(int,const char *);
    extern void sendauthchallenge(int,const char *);
    extern void sendauthreq(int, const char *);
    extern void signal_auth_success(int);
    extern void signal_auth_failure(int);
    
} //namespace server

#endif
