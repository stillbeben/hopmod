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
    
    extern int mastermode;
    extern int next_gamemode;
    extern string next_mapname;
    extern int next_gametime;
    
    extern void player_msg(int,const char *);
    extern void pausegame(bool);
    extern void kick(int cn,int time,const std::string & admin,const std::string & reason);
    extern void changetime(int remaining);
    extern void clearbans();
} //namespace server

#endif
