#ifdef INCLUDE_EXTSERVER_CPP

void player_msg(int cn,const char * text)
{
    get_ci(cn)->sendprivtext(text);
}

int player_sessionid(int cn){return get_ci(cn)->sessionid;}
const char * player_name(int cn){return get_ci(cn)->name;}
const char * player_team(int cn){return get_ci(cn)->team;}
const char * player_privilege(int cn)
{
    switch(get_ci(cn)->privilege)
    {
        case PRIV_MASTER: return "master";
        case PRIV_ADMIN: return "admin";
        default: return "none";
    }
}
int player_ping(int cn){return get_ci(cn)->ping;}
const char * player_ip(int cn){return get_ci(cn)->hostname();}
int player_iplong(int cn){return getclientip(get_ci(cn)->clientnum);}
const char * player_status(int cn)
{
    switch(get_ci(cn)->state.state)
    {
        case CS_ALIVE: return "alive";
        case CS_DEAD: return "dead"; 
        case CS_SPAWNING: return "spawning"; 
        case CS_LAGGED: return "lagged"; 
        case CS_SPECTATOR: return "spectator";
        case CS_EDITING: return "editing"; 
        default: return "unknown";
    }
}
int player_status_code(int cn){return get_ci(cn)->state.state;}
int player_connection_time(int cn)
{
    return (totalmillis - get_ci(cn)->connectmillis)/1000;
}

void changemap(const char * map,const char * mode = "",int mins = -1)
{
    int gmode = (mode[0] ? modecode(mode) : gamemode);
    if(!m_mp(gmode)) gmode = gamemode;
    sendf(-1, 1, "risii", SV_MAPCHANGE, map, gmode, 1);
    changemap(map,gmode,mins);
}

int getplayercount(){return clients.length();}

#endif
