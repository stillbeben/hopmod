--[[
    
    Copyright (C) 2009 Graham Daws
    
    MAINTAINER
        Graham
    
    OVERVIEW
    
        * Data is stored in a local table and is updated whenever a player
          leaves the game. At the end of the game, references to the data are
          given to the backend database objects for permanant storage.
          
        * Player data is associated by name and ip, if a player renames or
          changes IP address then a new record is created.
          
        * The player botskill field value 0 indicates that the player is human;
        * The player timeplayed field value is the number of seconds played;
        * The game duration field value is the number of minutes the game lasted.
        
        * Conditions for discarding all data:
            * Gamemode is coop-edit,
            * There are less than two ip-unique human players,
            * Game duration is less than one minute.
        
        * Conditions for discarding player data:
            * Zero frag count
        
    GUIDELINES
        * Define private functions in the internal table
        * CamelCase name convention (i.e. verbNoun) acceptable for temporary and private names
    
    CONTRIBUTIONS
        * server.stats_overwrite_name_with_authname feature by Zombie
    
    TODO
]]

require "geoip"

local game = nil
local players = nil
local internal = {}

local auth_domain

function internal.setNewGame()
    
    game = {
        datetime = os.time(),
        duration = server.timeleft,
        mode = server.gamemode,
        map = server.map,
        finished = false
    }
    
    players = {}
    -- players table will be populated by the addPlayer function which will be called on active event for each player
end

function internal.getPlayerTable(player_id)
    
    player_id = tonumber(player_id)
    
    if players[player_id] then 
        return players[player_id]
    end
    
    players[player_id] = {
        team_id     = 0,
        score       = 0, 
        frags       = 0, 
        deaths      = 0, 
        suicides    = 0, 
        hits        = 0, 
        misses      = 0,
        shots       = 0, 
        damage      = 0, 
        playing     = true, 
        timeplayed  = 0, 
        finished    = false, 
        win         = false, 
        rank        = 0, 
        country     = "", 
        botskill    = 0
    }
    
    if gamemodeinfo.ctf then
        players[player_id].flags_stolen = 0
        players[player_id].flags_scored = 0
    end
    
    return players[player_id]
end

function internal.getPlayerTableFromCn(cn)
    local player_id = server.player_id(cn)
    if players == nil or player_id == -1 then return nil end
    return internal.getPlayerTable(player_id)
end

function internal.updatePlayer(cn)
    
    --if server.player_vars(cn).block_stats then return {} end -- BROKEN from renaming event because of new id
    
    local player_id = server.player_id(cn)
    if players == nil or player_id == -1 then return {} end
    
    local t = internal.getPlayerTable(player_id)
    if not t then return {} end
    
    t.name = server.player_name(cn)
    t.team = server.player_team(cn)
    t.ipaddr = server.player_ip(cn)
    t.ipaddrlong = server.player_iplong(cn)
    t.country = geoip.ip_to_country_code(server.player_ip(cn))

    local frags = server.player_frags(cn)
    local suicides = server.player_suicides(cn)
    local teamkills = server.player_teamkills(cn)
    
    t.score = frags
    
    -- sauer bug: frags decremented for every suicide and teamkill
    frags = frags + suicides + teamkills
    t.frags = frags
    
    t.teamkills = server.player_teamkills(cn)
    t.suicides = suicides
    t.deaths = server.player_deaths(cn)
    t.hits = server.player_hits(cn)
    t.misses = server.player_misses(cn)
    t.shots = server.player_shots(cn)
    t.damage = server.player_damage(cn)
    t.damagewasted = server.player_damagewasted(cn)
    t.timeplayed = server.player_timeplayed(cn)
    
    return t
end

function internal.addPlayer(cn)

    local t = internal.updatePlayer(cn)
    
    local spec = server.player_status_code(cn) == server.SPECTATOR
    t.playing = not spec
    
    local human = not server.player_isbot(cn)
    
    if human and auth_domain then
        if server.player_vars(cn).stats_auth_name then
            t.auth_name = server.player_vars(cn).stats_auth_name
        else
            auth.send_auth_request(cn, auth_domain)
        end
    end
    
    return t
end

function internal.removePlayer(cn)
    local player_id = server.player_id(tonumber(cn))
    if players == nil or player_id == -1 then return end
    players[player_id] = nil
end

function internal.constructTeamsTable()
    
    if not gamemodeinfo.teams then return end
    
    local teams = {}
    
    for _, teamname in ipairs(server.teams()) do
    
        team = {}
        team.name = teamname
        team.score = server.team_score(teamname)
        team.win = server.team_win(teamname)
        team.draw = server.team_draw(teamname)
        
        table.insert(teams, team)
    end
    
    return teams
end

function internal.commit()
    
    if game == nil or players == nil then return end
    
    local function final_update(cn)
    
        local playerData = internal.updatePlayer(cn)
        
        playerData.finished = playerData.playing
        playerData.win = server.player_win(cn)
        playerData.rank = server.player_rank(cn)
        
        if auth_domain and playerData.auth_name then
            
            if server.stats_tell_auth_name == 1 then
                server.player_msg(cn, string.format("Saving your stats as %s@%s", playerData.auth_name, auth_domain))
            end
            
            if server.stats_overwrite_name_with_authname == 1 then
                playerData.player_name = playerData.name -- save the original name
                playerData.name = playerData.auth_name
            end
        end
    end
    
    -- Update stats for human players
    for _, cn in ipairs(server.players()) do
        final_update(cn)
    end
    
    -- Update stats for bot players
    for _, cn in ipairs(server.bots()) do
        final_update(cn)
    end

    local human_players = 0
    local bot_players = 0
    local unique_players = 0 -- human players
    local ipcount = {}
    
    -- Count the players
    for id, player in pairs(players) do
        
        if (player.frags == 0 and player.timeplayed < 60) then -- Remove players that didn't contribute to the game
            players[id] = nil
        else
            
            if player.botskill == 0 then
                human_players = human_players + 1
                
                if not ipcount[player.ipaddrlong] then
                    ipcount[player.ipaddrlong] = true
                    unique_players = unique_players + 1
                end
            else
                bot_players = bot_players + 1
            end
        end
    end
    
    local teams = internal.constructTeamsTable()
    
    game.players = human_players
    game.bots = bot_players
    game.duration = round(server.gamemillis / 60000)
    
    if unique_players < 2 or game.duration == 0 or not internal.supported_gamemodes[game.mode] then
        game = nil
        players = nil
        return
    end
    
    local query_backend = internal.backends.query
    internal.backends.query = nil
    
    for _, backend in pairs(internal.backends) do
        catch_error(backend.commit_game, game, players, teams)
    end
    
    internal.backends.query = query_backend
    
    game = nil
    players = nil
end

function internal.loadAuthHandlers(domain)

    local found_domain = auth.directory.get_domain(domain)

    if not found_domain then
        server.log_error("stats auth won't be supported: unknown domain " .. tostring(domain))
        return
    end

    auth_domain = domain
    
    local listener_id = auth.listener(auth_domain, function(cn, user_id, domain, status)
        
        if status ~= auth.request_status.SUCCESS then return end
        
        server.player_vars(cn).stats_auth_name = name
        
        local t = internal.getPlayerTable(server.player_id(cn))
        t.auth_name = name
        
        server.player_msg(cn, "You are logged in as " .. magenta(name) .. ".")
    end)

    function internal.unloadAuthHandlers()
        auth.cancel_listener(listener_id)
    end
end

function internal.loadEventHandlers()

    local active = server.event_handler("maploaded", internal.addPlayer)
    
    local disconnect = server.event_handler("disconnect", function(cn)
        internal.updatePlayer(cn).playing = false
        server.player_vars(cn).stats_auth_name = nil
    end)
    
    local addbot = server.event_handler("addbot", function(cn, skill, botcn)
        internal.addPlayer(botcn).botskill = skill
    end)
    
    local botleft = server.event_handler("botleft", function(botcn)
        internal.updatePlayer(botcn).playing = false
    end)
    
    local intermission = server.event_handler("intermission", function()
        if game then game.finished = true end
        internal.commit()
    end)
    
    local finishedgame = server.event_handler("finishedgame", internal.commit)
    local mapchange = server.event_handler("mapchange", internal.setNewGame)
    
    local _rename = server.event_handler("rename", function(cn)
        internal.addPlayer(cn)
    end)
    
    local renaming = server.event_handler("renaming", function(cn)
        internal.updatePlayer(cn).playing = false
    end)
    
    local spectator = server.event_handler("spectator", function(cn, value)
        internal.updatePlayer(cn).playing = (value == 0)
    end)
    
    local takeflag = server.event_handler("takeflag", function(cn)
        local player = internal.getPlayerTableFromCn(cn)
        player.flags_stolen = player.flags_stolen + 1
    end)
    
    local scoreflag = server.event_handler("scoreflag", function(cn)
        local player = internal.getPlayerTableFromCn(cn)
        player.flags_scored = player.flags_scored + 1
    end)

    function internal.unloadEventHandlers()
    
        server.cancel_handler(active)
        server.cancel_handler(disconnect)
        server.cancel_handler(addbot)
        server.cancel_handler(botleft)
        server.cancel_handler(intermission)
        server.cancel_handler(finishedgame)
        server.cancel_handler(_rename)
        server.cancel_handler(renaming)
        server.cancel_handler(spectator)
        server.cancel_handler(takeflag)
        server.cancel_handler(scoreflag)
        
    end
    
end

function internal.initialize(tableOfBackends, settings)
    
    internal.loadEventHandlers()
    
    if settings.using_auth == 1 then
        internal.loadAuthHandlers(settings.auth_domain)
    end
    
    internal.backends = tableOfBackends
    internal.supported_gamemodes = settings.gamemodes
end

function internal.shutdown()
    
    game = nil
    players = nil
    
    if internal.unloadEventHandlers then internal.unloadEventHandlers() end
    if internal.unloadAuthHandlers then internal.unloadAuthHandlers() end
    
    internal = nil
    server.shutdown_stats = nil
    
end

server.shutdown_stats = internal.shutdown

return {initialize = internal.initialize, shutdown = internal.shutdown}
