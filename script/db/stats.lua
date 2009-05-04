require "sqlite3"
dofile("./script/db/sqliteutils.lua")
require "Json"

local game = nil
local stats = nil
local evthandlers = {}
local statsmod = {} -- namespace for stats functions

local db = sqlite3.open(server.stats_db_filename)
createMissingTables("./script/db/stats_schema.sql", db)

if tonumber(server.stats_debug) == 1 then statsmod.db = db end

local perr
local insert_game,perr = db:prepare("INSERT INTO games (datetime, duration, gamemode, mapname, players, bots, finished) VALUES (:datetime, :duration, :mode, :map, :players, :bots, :finished)")
if not insert_game then error(perr) end

local insert_team,perr = db:prepare("INSERT INTO teams (game_id, name, score, win, draw) VALUES (:gameid,:name,:score,:win,:draw)")
if not insert_team then error(perr) end

local insert_player,perr = db:prepare[[INSERT INTO players (game_id, team_id, name, ipaddr, country, score, frags, deaths, suicides, teamkills, hits, shots, damage, damagewasted, timeplayed, finished, win, rank, botskill) 
 VALUES(:gameid, :team_id, :name, :ipaddr, :country, :score, :frags, :deaths, :suicides, :teamkills, :hits, :shots, :damage, :damagewasted, :timeplayed, :finished, :win, :rank, :botskill)]]
if not insert_player then error(perr) end

local select_player_totals,perr = db:prepare("SELECT * FROM playertotals WHERE name = :name")
if not select_player_totals then error(perr) end

local domain_id
local domain_name

local send_verified_msg = function(cn, name)
    server.msg(string.format("%s is verified.", green(name)))
end

function statsmod.setNewGame()
    game = {datetime = os.time(), duration = server.timeleft, mode = server.gamemode, map = server.map, finished = false}
    stats = {}
    -- addPlayer function will be called on active event for each player
end

function statsmod.getPlayerTable(player_id)
    player_id = tonumber(player_id)
    if stats[player_id] then return stats[player_id] end
    stats[player_id] = {team_id = 0, score = 0, frags = 0, deaths = 0, suicides = 0, hits = 0, shots = 0, damage = 0, playing = true, timeplayed = 0, finished = false, win = false, rank = 0, country = "", botskill = 0}
    return stats[player_id]
end

function statsmod.updatePlayer(cn)
    
    if server.player_pvars(cn).stats_block then return {} end
    
    local player_id = server.player_id(cn)
    if stats == nil or player_id == -1 then return {} end
    
    local t = statsmod.getPlayerTable(player_id)
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

function statsmod.isPlayerVerified(cn)
    local pvars = server.player_pvars(cn)
    return pvars.stats_id_verified and pvars.stats_id_name == server.player_name(cn) and tonumber(server.uptime) < pvars.stats_id_expire
end

function statsmod.addPlayer(cn)
    
    local human = not server.player_isbot(cn)
    
    if human and domain_id and auth.found_name(server.player_name(cn),domain_id) then
    
        if not statsmod.isPlayerVerified(cn) then
        
            local pvars = server.player_pvars(cn)
            pvars.stats_block = true
            
            server.sendauthreq(cn, domain_name)
            
            local sid = server.player_sessionid(cn)
            local pid = server.player_id(cn)
            
            server.player_msg(cn, "You are using a reserved name and have about 10 seconds to authenticate your ID.")
            
            server.sleep(13000, function()
                
                if sid ~= server.player_sessionid(cn) or pid ~= server.player_id(cn) then return end
                
                if not statsmod.isPlayerVerified(cn) then
                    server.kick(cn, 0, "server", "using reserved name")
                end
                
            end)
            
            return {}
        end
    end
    
    local t = statsmod.updatePlayer(cn)
    t.playing = true
    return t
end

function statsmod.commitStats()
    
    if stats == nil then return end
    
    local cpu_start = os.clock()
    local real_start = os.time()
    
    for i,cn in ipairs(server.players()) do 
        local t = statsmod.updatePlayer(cn)
        if t.playing then t.finished = true end
        t.win = server.player_win(cn)
        t.rank = server.player_rank(cn)
    end

    for i,cn in ipairs(server.bots()) do
        local t = statsmod.updatePlayer(cn)
        if t.playing and game.finished then t.finished = true end
    end
    
    local human_players = 0
    local bot_players = 0
    local unique_players = 0 -- human players
    local ipcount = {}
    for id,player in pairs(stats) do
    
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
    
    if unique_players < 2 or server.gamemode == "coop edit" or game.duration == 0 then
        stats = nil
        return
    end
    
    game.players = human_players
    game.bots = bot_players
    game.duration = game.duration - server.timeleft
    
    if tonumber(server.stats_use_json) == 1 then
        statsmod.writeStatsToJsonFile()
        return
    end
    
    if tonumber(server.stats_use_sqlite) == 0 then
        stats = nil
        return
    end
    
    db:exec("BEGIN TRANSACTION")
    
    insert_game:bind(game)
    insert_game:exec()
    local game_id = db:last_insert_rowid()
    
    if server.gamemodeinfo.teams then
        for i,teamname in ipairs(server.teams()) do
            
            team = {}
            team.gameid = game_id
            team.name = teamname
            team.score = server.team_score(teamname)
            team.win = server.team_win(teamname)
            team.draw = server.team_draw(teamname)
            
            insert_team:bind(team)
            insert_team:exec()
            
            local team_id = db:last_insert_rowid()
            local team_name = team.name
            
            --for i2,teamplayer in ipairs(server.team_players(teamname)) do
            --    statsmod.getPlayerTable(server.player_id(teamplayer)).team_id = team_id
            --end
            
            for id,player in pairs(stats) do
                if player.team == team_name then player.team_id = team_id end
            end
        end
    end
    
    for id,player in pairs(stats) do
        player.gameid = game_id
        
        insert_player:bind(player)
        insert_player:exec()
    end
    
    db:exec("COMMIT TRANSACTION")
    
    local cpu_time = os.clock() - cpu_start
    local real_time = os.time() - real_start
    
    if real_time > 0 or cpu_time > 0.1 then
        print(string.format("committed player stats to database: cpu %f secs, real %f secs.",cpu_time,real_time))
    end
    
    stats = nil
end

local function installHandlers()

    local connect = server.event_handler("active", statsmod.addPlayer)
    local disconnect = server.event_handler("disconnect", function(cn) statsmod.updatePlayer(cn).playing = false end)
    
    local addbot = server.event_handler("addbot", function(cn, skill, botcn)
        statsmod.addPlayer(botcn).botskill = skill
    end)
    
    local botleft = server.event_handler("botleft", function(botcn)
        statsmod.updatePlayer(botcn).playing = false
    end)
    
    local intermission = server.event_handler("intermission", function()
        game.finished = true
        statsmod.commitStats()
    end)
    
    local finishedgame = server.event_handler("finishedgame", statsmod.commitStats)
    local mapchange = server.event_handler("mapchange", statsmod.setNewGame)
    local _rename = server.event_handler("rename", statsmod.addPlayer)
    local renaming = server.event_handler("renaming", function(cn) statsmod.updatePlayer(cn).playing = false end)
    
    table.insert(evthandlers, connect)
    table.insert(evthandlers, disconnect)
    table.insert(evthandlers, addbot)
    table.insert(evthandlers, botleft)
    table.insert(evthandlers, intermission)
    table.insert(evthandlers, finishedgame)
    table.insert(evthandlers, mapchange)
    table.insert(evthandlers, _rename)
    table.insert(evthandlers, renaming)
    
    if tonumber(server.stats_use_auth) == 1 then
    
        local domId = auth.get_domain_id(server.stats_auth_domain)
        if not domId then error(string.format("stats auth domain '%s' not found",server.stats_auth_domain)) end
        domain_id = domId
        domain_name = server.stats_auth_domain
        
        auth_domain_handlers[domain_name] = function(cn, name)

            if name ~= server.player_name(cn) then
                server.player_msg(cn, string.format("You authenticated as %s but the server was expecting you as %s", green(name), green(server.player_name(cn))))
                return
            end

            local pvars = server.player_pvars(cn)
            pvars.stats_block = false
            pvars.stats_id_verified = true
            pvars.stats_id_name = name
            pvars.stats_id_expire = tonumber(server.uptime) + 86400
            
            send_verified_msg(cn, name)
            
            statsmod.addPlayer(cn)
        end
    end
    
end

local function uninstallHandlers()
    for i,handlerId in ipairs(evthandlers) do server.cancel_handler(handlerId) end
    evthandlers = {}
end

server.event_handler("mapchange", function()
    if tonumber(server.record_player_stats) == 1 then
        if #evthandlers == 0 then installHandlers() end
    else
        if #evthandlers > 0 then uninstallHandlers() end
    end
end)

server.event_handler("shutdown", function()
    db:close()
end)

function server.playercmd_stats(cn,selection)
    if not selection then
        local frags = server.player_frags(cn) + server.player_suicides(cn) + server.player_teamkills(cn)
        server.player_msg(cn,string.format("Score: %s Frags: %s Deaths: %s Accuracy %s",
            yellow(server.player_frags(cn)),
            green(frags),
            red(server.player_deaths(cn)),
            yellow(server.player_accuracy(cn).."%")))
        if server.gamemodeinfo.teams then
            server.player_msg(cn,string.format("Teamkills: %s",red(server.player_teamkills(cn))))
        end
    elseif selection == "total" then
        select_player_totals:bind{name = server.player_name(cn)}
        row = select_player_totals:first_row()
        if not row then
            server.player_msg(cn, "No stats found.")
            return
        end
        server.player_msg(cn, string.format("Games: %s Frags: %s Deaths: %s Wins: %s Losses: %s",yellow(row.games),green(row.frags),red(row.deaths),green(row.wins),red(row.losses)))
    end
end

function statsmod.writeStatsToJsonFile()

    local d = os.date("%0e%b%Y_%H:%M")
    local filename = string.format("log/game/%s_%s.json",d, game.map)
    
    local file = io.open(filename,"w");
    
    local root = {}
    root.game = game
    root.players = map_to_array(stats)
    
    if server.gamemodeinfo.teams then
        
        root.teams = {}
        
        for i,teamname in ipairs(server.teams()) do
            
            team = {}
            team.name = teamname
            team.score = server.team_score(teamname)
            team.win = server.team_win(teamname)
            team.draw = server.team_draw(teamname)
            
            table.insert(root.teams, team)
        end
    end
    
    file:write(Json.Encode(root))
    file:flush()
end

local find_names_by_ip = db:prepare("SELECT DISTINCT name FROM players WHERE ipaddr = :ipaddr")
if not find_names_by_ip then error(perr) end

function server.find_names_by_ip(ip)
    local names = {}
    find_names_by_ip:bind{ipaddr=ip}
    for row in find_names_by_ip:rows() do table.insert(names, row.name) end
    return names
end

function server.playercmd_showauth(cn)
    local pvars = server.player_pvars(cn)
    if pvars.stats_id_verified and pvars.stats_id_name == server.player_name(cn) then
        send_verified_msg(cn, server.player_name(cn))
    end
end

if tonumber(server.stats_debug) == 1 then return statsmod end

