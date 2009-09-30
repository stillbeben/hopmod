
require "luasql_mysql"

local servername
local connection

local function readWholeFile(filename)
    local file, err = io.open(filename)
    if not file then error(err) end
    return file:read("*a")
end

local function escape_string(s)
    s = string.gsub(s, "\\", "\\\\")
    s = string.gsub(s, "%\"", "\\\"")
    s = string.gsub(s, "%'", "\\'")
    return s
end

local function install_db(connection, settings)

    local schema = readWholeFile(settings.schema)

    for statement in string.gmatch(schema, "CREATE TABLE[^;]+") do
        local cursor, err = connection:execute(statement)
        if not cursor then error(err) end
    end
    
    local triggers = readWholeFile(settings.triggers)
    
    for statement in string.gmatch(triggers, "CREATE TRIGGER[^~]+") do
        local cursor, err = connection:execute(statement)
        if not cursor then error(err) end
    end
    
end

local function open(settings)
    
    connection = luasql.mysql():connect(settings.database, settings.username, settings.password, settings.hostname, settings.port)
    
    servername = settings.servername
    
    if settings.install then
        install_db(connection, settings)
    end
    
end

local function insert_game(game)

    local insert_game_sql = [[INSERT INTO games (servername, datetime, duration, gamemode, mapname, players, bots, finished) 
        VALUES ('%s', from_unixtime(%i), %i, '%s', '%s', %i, %i, %i)]]
    
    connection:execute(string.format(
        insert_game_sql,
        escape_string(servername),
        game.datetime,
        game.duration,
        escape_string(game.mode),
        escape_string(game.map),
        game.players,
        game.bots,
        game.finished and 1 or 0))
    
    return connection:execute("SELECT last_insert_id()"):fetch()
end

local function insert_team(game_id, team)
    
    local insert_team_sql = "INSERT INTO teams (game_id, name, score, win, draw) VALUES (%i, '%s', %i, %i, %i)"
    connection:execute(string.format(insert_team_sql, game_id, escape_string(team.name), team.score, team.win and 1 or 0, team.draw and 1 or 0))
    
    return connection:execute("SELECT last_insert_id()"):fetch()
end

local function insert_player(game_id, player)
    
    local insert_player_sql = [[INSERT INTO players 
        (game_id, team_id, name, ipaddr, country, score, frags, deaths, suicides, teamkills, hits, misses, shots, damage, damagewasted, timeplayed, finished, win, rank, botskill)
        VALUES(%i, %i, '%s', '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i, %i)]]
    
    connection:execute(string.format(
        insert_player_sql,
        game_id, 
        player.team_id or 0, 
        escape_string(player.name),
        player.ipaddr,
        player.country,
        player.score,
        player.frags,
        player.deaths,
        player.suicides,
        player.teamkills,
        player.hits,
        player.misses,
        player.shots,
        player.damage,
        player.damagewasted,
        player.timeplayed,
        player.finished and 1 or 0,
        player.win and 1 or 0,
        player.rank,
        player.botskill))
    
end

local function commit_game(game, players, teams)
    
    connection:execute("START TRANSACTION")
    
    local game_id = insert_game(game)
    
    for i, team in ipairs(teams or {}) do
        local team_id = insert_team(game_id, team)
        for id, player in pairs(players) do
            if player.team == team_name then player.team_id = team_id end
        end
    end
    
    for id, player in pairs(players) do
        insert_player(game_id, player)
    end
    
    connection:execute("COMMIT")
    
end

return {open = open, commit_game = commit_game}
