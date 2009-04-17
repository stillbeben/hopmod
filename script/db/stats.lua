require "sqlite3"
dofile("./script/db/sqliteutils.lua")
require "Json"

local game = nil
local stats = nil
local evthandlers = {}
statsmod = {} -- namespace for stats functions

local db = sqlite3.open(server.stats_db_filename)
createMissingTables("./script/db/stats_schema.sql", db)

local perr

local insert_game,perr = db:prepare("INSERT INTO games (datetime, duration, gamemode, mapname, players) VALUES (:datetime,:duration,:mode,:map,:players)")
if not insert_game then error(perr) end

local insert_team,perr = db:prepare("INSERT INTO teams (game_id, name, score, win, draw) VALUES (:gameid,:name,:score,:win,:draw)")
if not insert_team then error(perr) end

local insert_player,perr = db:prepare[[INSERT INTO players (game_id, team_id, name, ipaddr, frags, deaths, suicides, teamkills, hits, shots, damage, timeplayed, finished, win) 
    VALUES(:gameid,:team_id,:name,:ipaddr,:frags,:deaths,:suicides,:teamkills,:hits,:shots,:damage,:timeplayed,:finished,:win)]]
if not insert_player then error(perr) end

local select_player_totals,perr = db:prepare("SELECT * FROM playertotals WHERE name = :name")
if not select_player_totals then error(perr) end

function statsmod.setNewGame()
    game = {datetime = os.time(), duration = server.timeleft, mode = server.gamemode, map = server.map}
    stats = {}
end

function statsmod.getPlayerTable(player_id)
    player_id = tonumber(player_id)
    if stats[player_id] then return stats[player_id] end
    stats[player_id] = {team_id = 0, frags = 0, deaths = 0, suicides = 0, hits = 0, shots = 0, damage = 0, playing = true, timeplayed = 0, finished = false, won = false}
    return stats[player_id]
end

function statsmod.updatePlayer(cn)

    local player_id = server.player_id(cn)
    
    local t = statsmod.getPlayerTable(player_id)
    
    t.name = server.player_name(cn)
    if gamemodeinfo.teams then t.team = server.player_team(cn) end -- useful for stats serialized to json
    t.ipaddr = server.player_ip(cn)
    t.ipaddrlong = server.player_iplong(cn)
    
    local frags = server.player_frags(cn)
    local suicides = server.player_suicides(cn)
    
    -- sauer bug: frags decremented for every suicide
    frags = frags + suicides
    
    t.frags = frags
    t.teamkills = server.player_teamkills(cn)
    t.suicides = suicides
    t.deaths = server.player_deaths(cn)
    t.hits = server.player_hits(cn)
    t.shots = server.player_shots(cn)
    t.damage = server.player_damage(cn)
    t.timeplayed = server.player_timeplayed(cn)
    
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
    end
    
    game.players = #stats
    game.duration = game.duration - server.timeleft
    
    local unique_players = 0
    local ipcount = {}
    for id,player in pairs(stats) do
        if not ipcount[player.ipaddrlong] then
            ipcount[player.ipaddrlong] = true
            unique_players = unique_players + 1
        end
    end
    
    --if unique_players < 2 or server.gamemode == "coop edit" or game.duration == 0 then
    --    stats = nil
    --    return
    --end
    
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
    
    if gamemodeinfo.teams then
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
            
            for i2,teamplayer in ipairs(server.team_players(teamname)) do
                statsmod.getPlayerTable(server.player_id(teamplayer)).team_id = team_id
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
    
    if real_time > 0 or cpu_time > 0 then
        print(string.format("committed player stats to database: cpu %i secs, real %i secs.",cpu_time,real_time))
    end
    
    stats = nil
end

local function installHandlers()

    local connect = server.event_handler("connect", function(cn) statsmod.updatePlayer(cn).playing = true end)
    local disconnect = server.event_handler("disconnect", function(cn) statsmod.updatePlayer(cn).playing = false end)
    local intermission = server.event_handler("intermission", statsmod.commitStats)
    local finishedgame = server.event_handler("finishedgame", statsmod.commitStats)
    local mapchange = server.event_handler("mapchange", statsmod.setNewGame)
    local _rename = server.event_handler("rename", function(cn) statsmod.updatePlayer(cn).playing = true end)
    local renaming = server.event_handler("renaming", function(cn) statsmod.updatePlayer(cn).playing = false end)
    
    table.insert(evthandlers, connect)
    table.insert(evthandlers, disconnect)
    table.insert(evthandlers, intermission)
    table.insert(evthandlers, finishedgame)
    table.insert(evthandlers, mapchange)
    table.insert(evthandlers, _rename)
    table.insert(evthandlers, renaming)
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
        server.player_msg(cn,string.format("Frags: %s Deaths: %s Accuracy %s",green(server.player_frags(cn)),red(server.player_deaths(cn)),yellow(server.player_accuracy(cn).."%")))
        if gamemodeinfo.teams then
            server.player_msg(cn,string.format("Teamkills: %s",red(server.player_teamkills(cn))))
        end
    elseif selection == "total" then
        select_player_totals:bind{name = server.player_name(cn)}
        row = select_player_totals:first_row()
        if not row then
            server.player_msg(cn, "No stats found.")
            return
        end
        server.player_msg(cn, string.format("Games: %s Frags: %s Deaths: %s Wins: %s Losses: %s",yellow(row.games),green(row.frags),red(row.deaths),green(row.games),red(row.losses)))
    end
end

function statsmod.writeStatsToJsonFile()

    local d = os.date("%0e%b%Y_%H:%M")
    local filename = string.format("log/game/%s_%s.json",d, game.map)
    
    local file = io.open(filename,"w");
    
    local root = {}
    root.game = game
    root.players = map_to_array(stats)
    
    if gamemodeinfo.teams then
        
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
