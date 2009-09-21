
local game = nil
local players = nil
local internal = {}

local domain_id
local domain_name

function internal.setNewGame()
    game = {datetime = os.time(), duration = server.timeleft, mode = server.gamemode, map = server.map, finished = false}
    players = {}
    -- players table will be populated by the addPlayer function which will be called on active event for each player
end

function internal.getPlayerTable(player_id)
    player_id = tonumber(player_id)
    if players[player_id] then return players[player_id] end
    players[player_id] = {team_id = 0, score = 0, frags = 0, deaths = 0, 
        suicides = 0, hits = 0, shots = 0, damage = 0, playing = true, 
        timeplayed = 0, finished = false, win = false, rank = 0, 
        country = "", botskill = 0}
    return players[player_id]
end

function internal.updatePlayer(cn)
    
    if server.player_pvars(cn).players_block then return {} end
    
    local player_id = server.player_id(cn)
    if players == nil or player_id == -1 then return {} end
    
    local t = internal.getPlayerTable(player_id)
    if not t then return {} end
    
    t.name = server.player_name(cn)
    t.team = server.player_team(cn)
    t.ipaddr = server.player_ip(cn)
    t.ipaddrlong = server.player_iplong(cn)
    t.country = server.ip_to_country_code(server.player_ip(cn))

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
    t.shots = server.player_shots(cn)
    t.damage = server.player_damage(cn)
    t.damagewasted = server.player_damagewasted(cn)
    t.timeplayed = server.player_timeplayed(cn)
    
    return t
end

function internal.addPlayer(cn)

    local t = internal.updatePlayer(cn)
    t.playing = true
    
    local human = not server.player_isbot(cn)
    
    if human and domain_id then
        
        local pvars = server.player_pvars(cn)

        if pvars.players_auth_name then
            t.auth_name = pvars.players_auth_name
        else
            auth.sendauthreq(cn, domain_name)
        end
    end
    
    return t
end

function internal.commit()
    
    if game == nil or players == nil then return end
    
    for i, cn in ipairs(server.players()) do
        
        local t = internal.updatePlayer(cn)
        if t.playing then t.finished = true end
        
        t.win = server.player_win(cn)
        t.rank = server.player_rank(cn)
        
        if domain_name and t.auth_name then
            
            if server.stats_tell_auth_name == 1 then
                server.player_msg(cn, string.format("Saving your stats as %s@%s", t.auth_name, domain_name))
            end
            
            if server.stats_overwrite_name_with_authname == 1 then
                t.player_name = t.name -- save the original name
                t.name = t.auth_name
            end
            
        end
    end
    
    for i, cn in ipairs(server.bots()) do
        local t = internal.updatePlayer(cn)
        if t.playing and game.finished then t.finished = true end
    end
    
    local human_players = 0
    local bot_players = 0
    local unique_players = 0 -- human players
    local ipcount = {}
    
    for id, player in pairs(players) do
    
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
    
    --if unique_players < 2 or server.gamemode == "coop edit" or game.duration == 0 then
    --    stats = nil
    --    return
    --end
    
    game.players = human_players
    game.bots = bot_players
    game.duration = game.duration - server.timeleft
    
    for i, backend in pairs(internal.backends) do
        backend.commit_game(game, players)
    end
    
    game = nil
    players = nil
end

function internal.loadAuthHandlers(domain)
    
    domain_id = auth.get_domain_id(domain)
    if not domain_id then error(string.format("players auth domain '%s' not found", domain)) end
    
    domain_name = domain
    
    local handler_id = auth.add_domain_handler(domain_name, function(cn, name)
        
        server.player_pvars(cn).stats_auth_name = name
        server.player_vars(cn).stats_auth_name = name
        
        local t = internal.getPlayerTable(server.player_id(cn))
        t.auth_name = name
        
        server.player_msg(cn, "You are logged in as " .. magenta(name) .. ".")
    end)

    function internal.unloadAuthHandlers()
        auth.cancel_domain_handler(handler_id)
    end
end

function internal.loadEventHandlers()

    local active = server.event_handler("active", internal.addPlayer)
    
    local disconnect = server.event_handler("disconnect", function(cn) 
        internal.updatePlayer(cn).playing = false
        server.player_unsetpvar(cn,"players_auth_name")
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
        server.player_pvars(cn).players_auth_name = server.player_vars(cn).players_auth_name
        internal.addPlayer(cn)
    end)
    
    local renaming = server.event_handler("renaming", function(cn) 
        internal.updatePlayer(cn).playing = false
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
        
    end
    
end

function internal.initialize(tableOfBackends, settings)
    
    internal.loadEventHandlers()
    
    if settings.using_auth == 1 then
        internal.loadAuthHandlers(settings.auth_domain_name)
    end
    
    internal.backends = tableOfBackends

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
