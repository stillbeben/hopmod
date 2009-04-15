#ifdef INCLUDE_EXTSERVER_CPP

static boost::signals::connection close_listenserver_slot;
static bool reload = false;

void init_hopmod()
{
    init_scripting();
    close_listenserver_slot = signal_shutdown.connect(flushserverhost);
    signal_shutdown.connect(shutdown_scripting);
    
    register_server_script_bindings(get_script_env());
    register_signals(get_script_env());
    init_scheduler();
    init_script_pipe();
    open_script_pipe("serverexec",511,get_script_env());
    init_script_socket();
    
    try{fungu::script::execute_file(STARTUP_SCRIPT,get_script_env().get_global_scope());}
    catch(fungu::script::error_info * error){report_script_error(error);}
}

void reload_hopmod()
{
    if(!reload)
    {
        reload = true;
        return;
    }
    else reload = false;
    
    signal_reloadhopmod();
    
    close_listenserver_slot.block();
    signal_shutdown();
    close_listenserver_slot.unblock();
    
    disconnect_all_slots();
    
    init_hopmod();
}

void update_hopmod()
{
    if(reload) reload_hopmod();
    
    run_script_pipe_service(totalmillis);
    run_script_socket_service();
    update_scheduler(totalmillis);
    bantimes.update(totalmillis);
    cleanup_dead_slots();
}

void shutdown()
{
    signal_shutdown();
    exit(0);
}

struct kickinfo
{
    int cn;
    int sessionid;
    int time; //seconds
    std::string admin;
    std::string reason;
};

static int execute_kick(void * vinfoptr)
{
    kickinfo * info = (kickinfo *)vinfoptr;
    clientinfo * ci = (clientinfo *)getinfo(info->cn);
    
    if(!ci || ci->sessionid != info->sessionid)
    {
        delete info;
        return 0;
    }
    
    std::string full_reason;
    if(info->reason.length())
    {
        full_reason = (info->time == 0 ? "kicked for " : "kicked and banned for ");
        full_reason += info->reason;
    }
    ci->disconnect_reason = full_reason;
    
    allowedips.removeobj(getclientip(ci->clientnum));
    netmask addr(getclientip(ci->clientnum));
    if(info->time == -1) bannedips.set_permanent_ban(addr);
    else bantimes.set_ban(addr,info->time);
    
    signal_kick(info->cn, info->time, info->admin, info->reason);
    
    disconnect_client(info->cn, DISC_KICK);
    
    delete info;
    return 0;
}

void kick(int cn,int time,const std::string & admin,const std::string & reason)
{
    clientinfo * ci = get_ci(cn);
    
    kickinfo * info = new kickinfo;
    info->cn = cn;
    info->sessionid = ci->sessionid;
    info->time = time;
    info->admin = admin;
    info->reason = reason;
    
    sched_callback(&execute_kick, info);
}

void changetime(int remaining)
{
    gamelimit = gamemillis + remaining;
    if(!gamepaused) checkintermission();
}

void clearbans()
{
    bantimes.clear();
}

void player_msg(int cn,const char * text)
{
    get_ci(cn)->sendprivtext(text);
}

int player_id(int cn)
{
    clientinfo * ci = getinfo(cn);
    return (ci ? ci->playerid : -1);
}

int player_sessionid(int cn)
{
    clientinfo * ci = getinfo(cn);
    return (ci ? ci->sessionid : -1);
}

const char * player_name(int cn){return get_ci(cn)->name;}
const char * player_team(int cn){return get_ci(cn)->team;}
const char * player_ip(int cn){return get_ci(cn)->hostname();}
int player_iplong(int cn){return getclientip(get_ci(cn)->clientnum);}
int player_status_code(int cn){return get_ci(cn)->state.state;}
int player_ping(int cn){return get_ci(cn)->ping;}
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
int player_accuracy(int cn)
{
    clientinfo * ci = get_ci(cn);
    
    int hits = ci->state.hits;
    int shots = ci->state.shots;
    if(!shots) return 0;
    
    return static_cast<int>(roundf(static_cast<float>(hits)/shots*100));
}

int player_privilege_code(int cn){return get_ci(cn)->privilege;}

const char * player_privilege(int cn)
{
    switch(get_ci(cn)->privilege)
    {
        case PRIV_MASTER: return "master";
        case PRIV_ADMIN: return "admin";
        default: return "none";
    }
}

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

int player_connection_time(int cn)
{
    return (totalmillis - get_ci(cn)->connectmillis)/1000;
}

int player_timeplayed(int cn)
{
    clientinfo * ci = get_ci(cn);
    return (ci->state.timeplayed + (ci->state.state != CS_SPECTATOR ? (lastmillis - ci->state.lasttimeplayed) : 0))/1000;
}

int player_win(int cn)
{
    clientinfo * ci = get_ci(cn);
    loopv(clients)
        if(clients[i] != ci && clients[i]->state.state != CS_SPECTATOR && 
            clients[i]->state.aitype == AI_NONE)
        {
            if( (clients[i]->state.frags > ci->state.frags) ||
                (clients[i]->state.frags == ci->state.frags && clients[i]->state.deaths < clients[i]->state.deaths) ||
                (clients[i]->state.deaths == ci->state.deaths && player_accuracy(clients[i]->clientnum) > player_accuracy(cn)) ||
                true ) return false;
        }
    return true;
}

void player_slay(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(ci->state.state != CS_ALIVE) return;
    ci->state.state = CS_DEAD;
    sendf(-1, 1, "ri2", SV_FORCEDEATH, cn);
}

bool player_changeteam(int cn,const char * newteam)
{
    clientinfo * ci = get_ci(cn);
    if(!m_teammode || ci->state.state == CS_SPECTATOR || 
        (!smode || !smode->canchangeteam(ci, ci->team, newteam)) ||
        signal_chteamrequest(cn, ci->team, newteam) == -1) return false;
    
    if(smode) smode->changeteam(ci, ci->team, newteam);
    signal_reteam(ci->clientnum, ci->team, newteam);
    
    copystring(ci->team, newteam, MAXTEAMLEN+1);
    sendf(-1, 1, "riis", SV_SETTEAM, cn, newteam);
    
    if(ci->state.aitype == AI_NONE) aiman::dorefresh = true;
    
    return true;
}

void changemap(const char * map,const char * mode = "",int mins = -1)
{
    int gmode = (mode[0] ? modecode(mode) : gamemode);
    if(!m_mp(gmode)) gmode = gamemode;
    sendf(-1, 1, "risii", SV_MAPCHANGE, map, gmode, 1);
    changemap(map,gmode,mins);
}

int getplayercount()
{
    return numclients(-1, false, true);
}

int getbotcount()
{
    return numclients(-1, true, false) - numclients();
}

int getspeccount()
{
    return getplayercount() - numclients();
}

void team_msg(const char * team,const char * msg)
{
    if(!m_teammode) return;
    defformatstring(line)("server: " BLUE "%s",msg);
    loopv(clients)
    {
        clientinfo *t = clients[i];
        if(t->state.state==CS_SPECTATOR || t->state.aitype != AI_NONE || strcmp(t->team, team)) continue;
        t->sendprivtext(line);
    }
}

void player_spec(int cn)
{
    setspectator(get_ci(cn), true);
}

void player_unspec(int cn)
{
    setspectator(get_ci(cn), false);
}

int player_bots(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->bots.length();
}

void unsetmaster()
{
    if(currentmaster != -1)
    {
        clientinfo * master = getinfo(currentmaster);
        
        defformatstring(msg)("The server has revoked your %s privilege.",privname(master->privilege));
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
    if(player->privilege == priv) return;
    unsetmaster();
    player->privilege = priv;
    currentmaster = cn;
    masterupdate = true;
    
    defformatstring(msg)("The server has raised your privilege to %s.", privname(priv));
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

bool writebanlist(const char * filename)
{
    return write_banlist(&bannedips, filename);
}

bool loadbanlist(const char * filename)
{
    return load_banlist(filename, &bannedips);
}

void addpermban(const char * addrstr)
{
    netmask addr;
    try{addr = netmask::make(addrstr);}
    catch(std::bad_cast){throw fungu::script::error(fungu::script::BAD_CAST);}
    bannedips.set_permanent_ban(addr);
}

void unsetban(const char * addrstr)
{
    netmask addr;
    try{addr = netmask::make(addrstr);}
    catch(std::bad_cast){throw fungu::script::error(fungu::script::BAD_CAST);}
    bannedips.unset_ban(addr);
}

void delbot(int cn)
{
    clientinfo * ci = get_ci(cn);
    aiman::reqdel(ci);
}

void enable_master_auth(bool enable)
{
    mastermask = (enable ? mastermask & ~MM_AUTOAPPROVE : mastermask | MM_AUTOAPPROVE);
}

void update_mastermask()
{
    bool autoapprove = mastermask & MM_AUTOAPPROVE;
    mastermask &= ~(1<<MM_VETO) & ~(1<<MM_LOCKED) & ~(1<<MM_PRIVATE) & ~MM_AUTOAPPROVE;
    mastermask |= (allow_mm_veto << MM_VETO);
    mastermask |= (allow_mm_locked << MM_LOCKED);
    mastermask |= (allow_mm_private << MM_PRIVATE);
    if(autoapprove) mastermask |= MM_AUTOAPPROVE;
}

const char * gamemodename()
{
    return modename(gamemode,"unknown");
}

std::vector<int> cs_clients_list(bool (* clienttype)(clientinfo *))
{
    std::vector<int> result;
    result.reserve(clients.length());
    loopv(clients) if(clienttype(clients[i])) result.push_back(clients[i]->clientnum);
    return result;
}

int lua_clients_list(lua_State * L,bool (* clienttype)(clientinfo *))
{
    lua_newtable(L);
    int count = 0;
    
    loopv(clients) if(clienttype(clients[i]))
    {
        lua_pushinteger(L,++count);
        lua_pushinteger(L,clients[i]->clientnum);
        lua_settable(L, -3);
    }
    
    return 1;
}

bool is_player(clientinfo * ci){return ci->state.state != CS_SPECTATOR && ci->state.aitype == AI_NONE;}
bool is_spectator(clientinfo * ci){return ci->state.state == CS_SPECTATOR; /* bots can't be spectators*/}

std::vector<int> cs_player_list(){return cs_clients_list(&is_player);}
int lua_player_list(lua_State * L){return lua_clients_list(L, &is_player);}
std::vector<int> cs_spec_list(){return cs_clients_list(&is_spectator);}
int lua_spec_list(lua_State * L){return lua_clients_list(L, &is_spectator);}

std::vector<std::string> get_teams()
{
    std::set<std::string> teams;
    loopv(clients) teams.insert(clients[i]->team);
    std::vector<std::string> result;
    std::copy(teams.begin(),teams.end(),std::back_inserter(result));
    return result;
}

int lua_team_list(lua_State * L)
{
    lua_newtable(L);
    std::vector<std::string> teams = get_teams();
    int count = 0;
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        lua_pushinteger(L, ++count);
        lua_pushstring(L, it->c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int get_team_score(const char * team)
{
    int score = 0;
    if(smode) return smode->getteamscore(team);
    else loopv(clients)
        if(clients[i]->state.state != CS_SPECTATOR && !strcmp(clients[i]->team,team))
            score += clients[i]->state.frags;
    return score;
}

std::vector<int> get_team_players(const char * team)
{
    std::vector<int> result;
    loopv(clients)
        if(clients[i]->state.state != CS_SPECTATOR && clients[i]->state.aitype == AI_NONE && !strcmp(clients[i]->team,team))
            result.push_back(clients[i]->clientnum);
    return result;
}

int lua_team_players(lua_State * L)
{
    int argc = lua_gettop(L);
    if(argc < 1) return luaL_error(L,"missing team name argument");
    std::vector<int> players = get_team_players(lua_tostring(L,1));
    lua_newtable(L);
    int count = 0;
    for(std::vector<int>::iterator it = players.begin(); it != players.end(); ++it)
    {
        lua_pushinteger(L, ++count);
        lua_pushinteger(L, *it);
        lua_settable(L, -3);
    }
    return 1;
}

int team_win(const char * team)
{
    std::vector<std::string> teams = get_teams();
    int score = get_team_score(team);
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        if(*it == team) continue;
        if(get_team_score(it->c_str()) > score) return false;
    }
    return true;
}

int team_draw(const char * team)
{
    std::vector<std::string> teams = get_teams();
    int score = get_team_score(team);
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        if(*it == team) continue;
        if(get_team_score(it->c_str()) != score) return false;
    }
    return true;
}

int recorddemo()
{
    if(demorecord) return demo_id;
    else return setupdemorecord(false);
}

int lua_gamemodeinfo(lua_State * L)
{
    lua_newtable(L);
    
    #define INFO_FIELD(m, name) \
        lua_pushboolean(L, m); \
        lua_setfield(L, -2, name) 
    INFO_FIELD(m_noitems, "noitems");
    INFO_FIELD(m_noammo, "noammo");
    INFO_FIELD(m_insta, "insta");
    INFO_FIELD(m_tactics, "tactics");
    INFO_FIELD(m_efficiency, "efficiency");
    INFO_FIELD(m_capture, "capture");
    INFO_FIELD(m_regencapture, "regencapture");
    INFO_FIELD(m_ctf, "ctf");
    INFO_FIELD(m_protect, "protect");
    INFO_FIELD(m_teammode, "teams");
    INFO_FIELD(m_overtime, "overtime");
    INFO_FIELD(m_timed, "timed");
    #undef INFO_FIELD
    
    return 1;
}

bool selectnextgame()
{
    signal_setnextgame();
    if(next_gamemode[0] && next_mapname[0])
    {
        int next_gamemode_code = modecode(next_gamemode);
        if(m_mp(next_gamemode_code))
        {
            mapreload = false;
            sendf(-1, 1, "risii", SV_MAPCHANGE, next_mapname, next_gamemode_code, 1);
            changemap(next_mapname, next_gamemode_code, next_gametime);
            next_gamemode[0] = '\0';
            next_mapname[0] = '\0';
            next_gametime = -1;
        }
        else
        {
            std::cerr<<next_gamemode<<" game mode is unrecognised."<<std::endl;
            sendf(-1, 1, "ri", SV_MAPRELOAD);
        }
        return true;
    }else return false;
}

#endif
