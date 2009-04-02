#ifndef HOPMOD_EXTAPI_HPP
#define HOPMOD_EXTAPI_HPP

#include <string>

namespace server
{
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
    
    extern int player_sessionid(int);
    extern void player_msg(int,const char *);
    extern const char * player_name(int);
    extern const char * player_team(int);
    extern const char * player_privilege(int);
    extern int player_ping(int);
    extern const char * player_ip(int);
    extern int player_iplong(int);
    extern const char * player_status(int);
    extern int player_status_code(int);
    extern int player_frags(int);
    extern int player_deaths(int);
    extern int player_suicides(int);
    extern int player_teamkills(int);
    extern int player_damage(int);
    extern int player_maxhealth(int);
    extern int player_health(int);
    extern int player_gun(int);
    extern int player_hits(int);
    extern int player_shots(int);
    extern int player_connection_time(int);
    extern void team_msg(const char *,const char *);
    extern void player_spec(int);
    extern void player_unspec(int);
    extern void unsetmaster();
    extern void server_setmaster(int);
    extern void server_setadmin(int);
    
    extern void pausegame(bool);
    extern void kick(int cn,int time,const std::string & admin,const std::string & reason);
    extern void changetime(int remaining);
    extern void clearbans();
    extern void changemap(const char * map,const char * mode,int mins);
    extern int modecode(const char * modename);
    extern int getplayercount();
} //namespace server

#endif
