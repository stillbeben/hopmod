/*
    This file is included from and is a part of "fpsgame/server.cpp".
*/
#ifdef INCLUDE_EXTSERVER_CPP

int sv_text_hit_length = 0;
int sv_sayteam_hit_length = 0;
int sv_mapvote_hit_length = 0;
int sv_switchname_hit_length = 0;
int sv_switchteam_hit_length = 0;
int sv_kick_hit_length = 0;
int sv_remip_hit_length = 0;
int sv_newmap_hit_length = 0;

bool kick_bannedip_group = true;

string authserver_hostname = "";

void authserver_reactor(masterserver_client & client, const char * reply, int argc, const char * const * argv)
{
    if(argc == 0) return;
    
    uint id = static_cast<uint>(atoi(argv[0]));
    
    if(strcmp(reply, "chalauth") == 0)
    {
        if(argc > 1) authchallenged(id, argv[1]);
    }
    else if(strcmp(reply, "succauth") == 0) authsucceeded(id);
    else if(strcmp(reply, "failauth") == 0) authfailed(id);
}

masterserver_client authserver(authserver_reactor);

masterserver_client & connect_to_authserver()
{
    if(!authserver.is_connected())
    {
        ENetAddress address;
        
        if( enet_address_set_host(&address, authserver_hostname) < 0)
        {
            std::cerr<<"Could not resolve hostname "<<authserver_hostname<<" for auth server connection."<<std::endl;
            return authserver;
        }
        
        address.port = masterport();
        
        if(authserver.connect(address) == false)
        {
            std::cerr<<"Unable to connect to auth server at "<<authserver_hostname<<":"<<address.port<<std::endl;
            return authserver;
        }
    }
    
    return authserver;
}

void check_authserver()
{
    if(authserver.is_connected())
    {
        static ENetSocketSet sockset;
        ENET_SOCKETSET_EMPTY(sockset);
        ENetSocket sock = authserver.get_socket_descriptor();
        ENET_SOCKETSET_ADD(sockset, sock);
        
        int status = enet_socketset_select(sock, &sockset, NULL, 0);
        
        if(status == 1)
            authserver.flush_input();
        else if(status == -1) authserver.disconnect();
    }
    
    if(authserver.is_connected() && authserver.has_queued_output())
        authserver.flush_output();
}

struct restore_state_header
{
    int gamemode;
    string mapname;
    
    int gamemillis;
    int gamelimit;
    
    int numscores;
    int numteamscores;
};

struct restore_teamscore
{
    string name;
    int score;
};

void crash_handler(int signal)
{
    unlink_script_pipe();
    
    int fd = open("log/restore", O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
    
    if(fd != -1)
    {
        restore_state_header header;
        
        header.gamemode = gamemode;
        copystring(header.mapname, smapname);
        
        header.gamemillis = gamemillis;
        header.gamelimit = gamelimit;
        
        header.numscores = scores.length() + clients.length();
        
        bool write_teamscores = false;
        header.numteamscores = 0;
        
        //the last four game modes have fixed team names and have simple team state
        if(gamemode+3 >= NUMGAMEMODES - 4 && smode)
        {
            header.numteamscores = 2;
            write_teamscores = true;
        }
        
        write(fd, &header, sizeof(header));
        
        write(fd, scores.buf, scores.length() * sizeof(savedscore));
        
        loopv(clients)
        {
            savedscore score;
            
            copystring(score.name, clients[i]->name);
            score.ip = getclientip(clients[i]->clientnum);
            
            score.save(clients[i]->state);
            
            write(fd, &score, sizeof(score));
        }
        
        if(write_teamscores)
        {
            restore_teamscore team;
            
            copystring(team.name, "evil");
            team.score = smode->getteamscore("evil");
            write(fd, &team, sizeof(team));
            
            copystring(team.name, "good");
            team.score = smode->getteamscore("good");
            write(fd, &team, sizeof(team));
        }
        
        close(fd);
    }
    
    int maxfd = getdtablesize();
    for(int i = 3; i < maxfd; i++) close(i);
    
    if(totalmillis > 10000)
    {
        //restart program from child process so a core dump is written for the failed parent process
        if(fork()!=0)
        {
            setpgid(0,0);
            execv(prog_argv[0], prog_argv);
        }
    }

}

void restore_server(const char * filename)
{
    int fd = open(filename, O_RDONLY, 0);
    if(fd == -1) return;
    
    restore_state_header header;
    
    int readlen = read(fd, &header, sizeof(header));
    
    if(readlen == -1)
    {
        close(fd);
        return;
    }
    
    int mins = (header.gamelimit - header.gamemillis)/60000;
    changemap(header.mapname, header.gamemode, mins);
    
    gamemillis = header.gamemillis;
    gamelimit = header.gamelimit;
    
    for(int i = 0; i < header.numscores; i++)
    {
        savedscore playerscore;
        readlen = read(fd, &playerscore, sizeof(savedscore));
        if(readlen == -1)
        {
            close(fd);
            return;
        }
        scores.add(playerscore);
    }
    
    for(int i = 0; i< header.numteamscores && smode; i++)
    {
        restore_teamscore team;
        readlen = read(fd, &team, sizeof(team));
        if(readlen == -1)
        {
            close(fd);
            return;
        }
        smode->setteamscore(team.name, team.score);
    }
    
    close(fd);
    unlink(filename);
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
    
    uint ip = getclientip(ci->clientnum);
    
    allowedips.removeobj(ip);
    
    netmask addrmask(ip);
    if(info->time > 0) bantimes.set_ban(addrmask, info->time);
    else if(info->time == -1) bannedips.set_permanent_ban(addrmask);
    
    signal_kick(info->cn, info->time, info->admin, info->reason);
    
    disconnect_client(info->cn, DISC_KICK);
    
    if(kick_bannedip_group && info->time > 0)
    {
        loopv(clients)
        {
            if(getclientip(clients[i]->clientnum) == ip) 
                kick(clients[i]->clientnum, 0, info->admin, "banned ip"); 
        }
    }
    
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

std::string player_displayname(int cn)
{
    clientinfo * ci = get_ci(cn);
    
    std::string output;
    output.reserve(MAXNAMELEN + 5);
    
    output = ci->name;
    
    bool is_bot = ci->state.aitype != AI_NONE;
    bool duplicate = false;
    
    if(!is_bot)
    {
        loopv(clients)
        {
            if(clients[i]->clientnum == cn) continue;
            if(!strcmp(clients[i]->name, ci->name))
            {
                duplicate = true;
                break;
            }
        }
    }
    
    if(is_bot || duplicate)
    {
        char open = (is_bot ? '[' : '(');
        char close = (is_bot ? ']' : ')');
        
        output += "\fs" MAGENTA " ";
        output += open;
        output += boost::lexical_cast<std::string>(cn);
        output += close;
        output += "\fr";
    }
    
    return output;
}

const char * player_team(int cn)
{
    return get_ci(cn)->team;
}

const char * player_ip(int cn)
{
    return get_ci(cn)->hostname();
}

int player_iplong(int cn)
{
    return getclientip(get_ci(cn)->clientnum);
}

int player_status_code(int cn)
{
    return get_ci(cn)->state.state;
}

int player_ping(int cn)
{
    return get_ci(cn)->ping;
}

int player_lag(int cn)
{
    return get_ci(cn)->lag;
}

int player_frags(int cn)
{
    return get_ci(cn)->state.frags;
}

int player_deaths(int cn)
{
    return get_ci(cn)->state.deaths;
}

int player_suicides(int cn)
{
    return get_ci(cn)->state.suicides;
}

int player_teamkills(int cn)
{
    return get_ci(cn)->state.teamkills;
}

int player_damage(int cn)
{
    return get_ci(cn)->state.damage;
}

int player_damagewasted(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->state.explosivedamage + ci->state.shotdamage - ci->state.damage;
}

int player_maxhealth(int cn)
{
    return get_ci(cn)->state.maxhealth;
}

int player_health(int cn)
{
    return get_ci(cn)->state.health;
}

int player_gun(int cn)
{
    return get_ci(cn)->state.gunselect;
}

int player_hits(int cn)
{
    return get_ci(cn)->state.hits;
}

int player_misses(int cn)
{
    return get_ci(cn)->state.misses;
}

int player_shots(int cn)
{
    return get_ci(cn)->state.shots;
}

int player_accuracy(int cn)
{
    clientinfo * ci = get_ci(cn);
    int shots = ci->state.shots;
    int hits = shots - ci->state.misses;
    return static_cast<int>(roundf(static_cast<float>(hits)/std::max(shots,1)*100));
}

int player_privilege_code(int cn)
{
    return get_ci(cn)->privilege;
}

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
    
    if(!m_teammode)
    {
        loopv(clients)
        {
            if(clients[i] == ci || clients[i]->state.state == CS_SPECTATOR) continue;
            
            bool more_frags = clients[i]->state.frags > ci->state.frags;
            bool eq_frags = clients[i]->state.frags == ci->state.frags;
            
            bool less_deaths = clients[i]->state.deaths < ci->state.deaths;
            bool eq_deaths = clients[i]->state.deaths == ci->state.deaths;
            
            int p1_acc = player_accuracy(clients[i]->clientnum);
            int p2_acc = player_accuracy(cn);
            
            bool better_acc = p1_acc > p2_acc;
            bool eq_acc = p1_acc == p2_acc;
            
            bool lower_ping = clients[i]->ping < ci->ping;
            
            if( more_frags || (eq_frags && less_deaths) ||
                (eq_frags && eq_deaths && better_acc) || 
                (eq_frags && eq_deaths && eq_acc && lower_ping)) return false;            
        }
        return true;
    }
    else return team_win(ci->team);
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
        (smode && !smode->canchangeteam(ci, ci->team, newteam)) ||
        signal_chteamrequest(cn, ci->team, newteam) == -1) return false;
    
    if(smode) smode->changeteam(ci, ci->team, newteam);
    signal_reteam(ci->clientnum, ci->team, newteam);
    
    copystring(ci->team, newteam, MAXTEAMLEN+1);
    sendf(-1, 1, "riis", SV_SETTEAM, cn, newteam);
    
    if(ci->state.aitype == AI_NONE) aiman::dorefresh = true;
    
    return true;
}

int player_authreq(int cn){return get_ci(cn)->authreq;}

int player_rank(int cn){return get_ci(cn)->rank;}
bool player_isbot(int cn){return get_ci(cn)->state.aitype != AI_NONE;}

int player_mapcrc(int cn){return get_ci(cn)->mapcrc;}

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

int player_pos(lua_State * L)
{
    int cn = luaL_checkint(L,1);
    vec pos = get_ci(cn)->state.o;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

std::vector<float> player_pos(int cn)
{
    vec pos = get_ci(cn)->state.o;
    std::vector<float> result(3);
    result[0] = pos.x;
    result[1] = pos.y;
    result[2] = pos.z;
    return result;
}

void cleanup_masterstate(clientinfo * master)
{
    int cn = master->clientnum;
    
    if(cn == mastermode_owner)
    {
        mastermode = MM_OPEN;
        mastermode_owner = -1;
        mastermode_mtime = totalmillis;
        allowedips.setsize(0);
    }
    
    if(gamepaused && cn == pausegame_owner) pausegame(false);
    
    if(master->state.state==CS_SPECTATOR) aiman::removeai(master);
}

void unsetmaster()
{
    if(currentmaster != -1)
    {
        clientinfo * master = getinfo(currentmaster);
        
        defformatstring(msg)("The server has revoked your %s privilege.",privname(master->privilege));
        master->sendprivtext(msg);
        
        master->privilege = 0;
        currentmaster = -1;
        masterupdate = true;
        
        cleanup_masterstate(master);
    }
}

void setpriv(int cn, int priv)
{
    clientinfo * player = get_ci(cn);
    if(player->privilege == priv || priv == PRIV_NONE) return;
    
    unsetmaster();
    
    const char * change = (priv > player->privilege ? "raised" : "lowered");
    
    player->privilege = priv;
    currentmaster = cn;
    masterupdate = true;
    
    defformatstring(msg)("The server has %s your privilege to %s.", change, privname(priv));
    player->sendprivtext(msg);
}

bool server_setmaster(int cn)
{
    if(!allow_master) return false;
    setpriv(cn, PRIV_MASTER);
    return true;
}

void server_setadmin(int cn)
{
    setpriv(cn, PRIV_ADMIN);
}

void set_invadmin(int cn)
{
    clientinfo * ci = get_ci(cn);
    ci->privilege = PRIV_ADMIN;
    sendf(ci->clientnum, 1, "ri3", SV_CURRENTMASTER, ci->clientnum, PRIV_ADMIN);
}

void unset_invadmin(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(ci->privilege != PRIV_ADMIN || cn == currentmaster) return;
    
    ci->privilege = PRIV_NONE;
    sendf(ci->clientnum, 1, "ri3", SV_CURRENTMASTER, ci->clientnum, PRIV_NONE);
    
    cleanup_masterstate(ci);
}

void addpermban(const char * addrstr)
{
    netmask addr;
    try{addr = netmask::make(addrstr);}
    catch(std::bad_cast){throw fungu::script::error(fungu::script::BAD_CAST);}
    bannedips.set_permanent_ban(addr);
}

int unsetban(const char * addrstr)
{
    netmask addr;
    try{addr = netmask::make(addrstr);}
    catch(std::bad_cast){throw fungu::script::error(fungu::script::BAD_CAST);}
    return bannedips.unset_ban(addr);
}

int addbot(int skill)
{
    clientinfo * owner = aiman::addai(skill, -1);
    if(!owner) return -1;
    signal_addbot(-1, skill, owner->clientnum);
    return owner->clientnum;
}

void enable_master_auth(bool enable)
{
    mastermask = (enable ? mastermask & ~MM_AUTOAPPROVE : mastermask | MM_AUTOAPPROVE);
}

bool using_master_auth()
{
    return !(mastermask & MM_AUTOAPPROVE);
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
bool is_bot(clientinfo * ci){return ci->state.aitype != AI_NONE;}

std::vector<int> cs_player_list(){return cs_clients_list(&is_player);}
int lua_player_list(lua_State * L){return lua_clients_list(L, &is_player);}
std::vector<int> cs_spec_list(){return cs_clients_list(&is_spectator);}
int lua_spec_list(lua_State * L){return lua_clients_list(L, &is_spectator);}
std::vector<int> cs_bot_list(){return cs_clients_list(&is_bot);}
int lua_bot_list(lua_State *L){return lua_clients_list(L, &is_bot);}

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
    std::vector<int> players = get_team_players(luaL_checkstring(L,1));
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

int recorddemo(const char * filename)
{
    if(demorecord) return demo_id;
    else return setupdemorecord(false, filename);
}

int lua_gamemodeinfo(lua_State * L)
{
    lua_newtable(L);
    
    lua_pushboolean(L, m_noitems);
    lua_setfield(L, -2, "noitems");
    
    lua_pushboolean(L, m_noammo);
    lua_setfield(L, -2, "noammo");
    
    lua_pushboolean(L, m_insta);
    lua_setfield(L, -2, "insta");
    
    lua_pushboolean(L, m_tactics);
    lua_setfield(L, -2, "tactics");
    
    lua_pushboolean(L, m_efficiency);
    lua_setfield(L, -2, "efficiency");
    
    lua_pushboolean(L, m_capture);
    lua_setfield(L, -2, "capture");
    
    lua_pushboolean(L, m_regencapture);
    lua_setfield(L, -2, "regencapture");
    
    lua_pushboolean(L, m_ctf);
    lua_setfield(L, -2, "ctf");
    
    lua_pushboolean(L, m_protect);
    lua_setfield(L, -2, "protect");
    
    lua_pushboolean(L, m_teammode);
    lua_setfield(L, -2, "teams");
    
    lua_pushboolean(L, m_overtime);
    lua_setfield(L, -2, "overtime");
    
    lua_pushboolean(L, m_timed);
    lua_setfield(L, -2, "timed");
    
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

bool delegateauth(int cn, const char * domain)
{
    clientinfo * ci = get_ci(cn);
    
    masterserver_client & authserver = connect_to_authserver();
    
    if(authserver.is_connected()==false)
    {
        ci->authreq = 0;
        sendf(ci->clientnum, 1, "ris", SV_SERVMSG, "not connected to authentication server");
        return false;
    }
    
    const char * args[4];
    defformatstring(id)("%i", ci->authreq);
    
    args[0] = id;
    args[1] = ci->authname;
    args[2] = (domain[0] == '\0' ? NULL : domain);
    args[3] = NULL;
    
    authserver.send_request("reqauth", args);
    
    return true;
}

bool relayauthanswer(int cn, const char * ans)
{
    clientinfo * ci = get_ci(cn);
    
    masterserver_client & authserver = connect_to_authserver();
    
    if(authserver.is_connected()==false)
    {
        ci->authreq = 0;
        sendf(ci->clientnum, 1, "ris", SV_SERVMSG, "not connected to authentication server");
        return false;
    }
    
    const char * args[3];
    defformatstring(id)("%i", ci->authreq);
    
    args[0] = id;
    args[1] = ans;
    args[2] = NULL;
    
    authserver.send_request("confauth", args);
    
    return true;
}

void sendauthchallenge(int cn, const char * challenge)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "risis", SV_AUTHCHAL, ci->authdomain, ci->authreq, challenge);
}

void sendauthreq(int cn, const char * domain)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "ris", SV_REQAUTH, domain);
}

void signal_auth_success(int cn, int id)
{
    clientinfo * ci = get_ci(cn);
    signal_auth(ci->clientnum, id, ci->authname, ci->authdomain, true);
    ci->authreq = 0;
}

void signal_auth_failure(int cn, int id)
{
    clientinfo * ci = get_ci(cn);
    signal_auth(ci->clientnum, id, ci->authname, ci->authdomain, false);
    ci->authreq = 0;
}

static bool compare_player_score(const std::pair<int,int> & x, const std::pair<int,int> & y)
{
    return x.first > y.first;
}

static void calc_player_ranks(const char * team)
{
    if(m_edit) return;
    
    if(m_teammode && !team)
    {
        std::vector<std::string> teams = get_teams();
        for(std::vector<std::string>::const_iterator it = teams.begin();
             it != teams.end(); it++) calc_player_ranks(it->c_str());
        return;
    }
    
    std::vector<std::pair<int,int> > players;
    players.reserve(clients.length());
    
    loopv(clients) 
        if(clients[i]->state.state != CS_SPECTATOR && (!team || !strcmp(clients[i]->team,team)))
            players.push_back(std::pair<int,int>(clients[i]->state.frags, i));
    
    std::sort(players.begin(), players.end(), compare_player_score);
    
    int rank = 0;
    for(std::vector<std::pair<int,int> >::const_iterator it = players.begin();
        it != players.end(); ++it)
    {
        rank++;
        if(it != players.begin() && it->first == (it-1)->first) rank--;
        clients[it->second]->rank = rank;
    }
}

void calc_player_ranks()
{
    return calc_player_ranks(NULL);
}

void script_set_mastermode(int value)
{
    mastermode = value;
    mastermode_owner = -1;
    mastermode_mtime = totalmillis;
    allowedips.setsize(0);
    if(mastermode >= MM_PRIVATE)
    {
        loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
    }
}

void add_allowed_ip(const char * ip)
{
    allowedips.add(inet_addr(ip));
}

void suicide(int cn)
{
    suicide(get_ci(cn));
}

bool compare_admin_password(const char * x)
{
    return !strcmp(x, masterpass);
}

std::vector<std::string> get_bans()
{
    std::vector<netmask> permbans = bannedips.get_permanent_bans();
    std::vector<netmask> tmpbans = bannedips.get_temporary_bans();
    std::vector<netmask> allbans;
    
    std::set_union(permbans.begin(), permbans.end(), tmpbans.begin(), tmpbans.end(), 
        std::inserter(allbans, allbans.begin()));
    
    std::vector<std::string> result;
    result.reserve(allbans.size());
    
    for(std::vector<netmask>::const_iterator it = allbans.begin(); it != allbans.end(); it++)
        result.push_back(it->to_string());
    
    return result;
}

#endif
