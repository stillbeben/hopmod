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

int player_frags(int cn){return get_ci(cn)->state.frags;}
int player_deaths(int cn){return get_ci(cn)->state.deaths;}
int player_suicides(int cn){return get_ci(cn)->state.suicides;}
int player_teamkills(int cn){return get_ci(cn)->state.teamkills;}
int player_damage(int cn){return get_ci(cn)->state.damage;}
int player_maxhealth(int cn){return get_ci(cn)->state.maxhealth;}
int player_health(int cn){return get_ci(cn)->state.health;}
int player_gun(int cn){return get_ci(cn)->state.gunselect;}
int player_hits(int cn){return get_ci(cn)->state.hits;}
int player_shots(int cn){return get_ci(cn)->state.shots;}

void changemap(const char * map,const char * mode = "",int mins = -1)
{
    int gmode = (mode[0] ? modecode(mode) : gamemode);
    if(!m_mp(gmode)) gmode = gamemode;
    sendf(-1, 1, "risii", SV_MAPCHANGE, map, gmode, 1);
    changemap(map,gmode,mins);
}

int getplayercount(){return clients.length();}

void team_msg(const char * team,const char * msg)
{
    if(!m_teammode) return;
    s_sprintfd(line)("server: " BLUE "%s",msg);
    loopv(clients)
    {
        clientinfo *t = clients[i];
        if(t->state.state==CS_SPECTATOR || t->state.aitype != AI_NONE || strcmp(t->team, team)) continue;
        t->sendprivtext(line);
    }
}

void player_spec(int cn){setspectator(get_ci(cn), true);}
void player_unspec(int cn){setspectator(get_ci(cn), false);}

void unsetmaster()
{
    if(currentmaster != -1)
    {
        clientinfo * master = getinfo(currentmaster);
        
        s_sprintfd(msg)("The server has revoked your %s privilege.",privname(master->privilege));
        master->sendprivtext(msg);
        
        master->privilege = 0;
        mastermode = MM_OPEN;
        allowedips.setsize(0);
        currentmaster = -1;
        masterupdate = true;
    }
}

void setpriv(int cn,int priv,bool hidden)
{
    clientinfo * player = get_ci(cn);
    unsetmaster();
    player->privilege = priv;
    currentmaster = cn;
    masterupdate = true;
    
    s_sprintfd(msg)("The server has raised your privilege to %s.", privname(priv));
    player->sendprivtext(msg);
}

void server_setmaster(int cn)
{
    setpriv(cn, PRIV_MASTER, false);
}

void server_setadmin(int cn)
{
    setpriv(cn, PRIV_ADMIN, false);
}

#endif
