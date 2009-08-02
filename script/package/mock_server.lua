
local setmetatable = setmetatable
local rawset = rawset
local string = require("string")
local pairs = pairs
local table = require("table")
local print = print

module("server")

player = {}
team = {}
gamemodeinfo = {}

function newPlayer(name, ip)
    
    local mt = {}
    mt.__newindex = function(t, key, value)
        
        if not _M["player_" .. key] then
            _M["player_" .. key] = function(cn)
                return player[cn][key]
            end
        end
        
        rawset(t, key, value)
    end
    
    local cn = #player + 1
    player[cn] = {}
    local pt = player[cn]
    setmetatable(pt, mt)
    
    pt.name = name
    pt.ip = ip
    pt.id = cn
    pt.cn = cn
    
    return pt
end

function removePlayer(cn)
    player[cn] = nil
end

function newTeam(name)
    
    local mt = {}
    mt.__newindex = function(t, key, value)
        
        if not _M["team_" .. key] then
            _M["team_" .. key] = function(name)
                return team[name][key]
            end
        end
        
        rawset(t, key, value)
    end
    
    team[name] = {}
    local tt = team[name]
    setmetatable(tt, mt)
    return tt
end

function initPlayerStats(t)
    t.frags = 0
    t.deaths = 0
    t.suicides = 0
    t.teamkills = 0
    t.hits = 0
    t.shots = 0
    t.damage = 0
    t.timeplayed = 0
end

function players()
    local array = {}
    for i,v in pairs(player) do
        if v then table.insert(array, i) end
    end
    return array
end

function teams()
    local array = {}
    for i,v in pairs(team) do
        if v then table.insert(array,i) end
    end
    return array
end

function team_players(name)
    local array = {}
    for i,v in pairs(player) do
        if v and v.team == name then
            table.insert(array,i)
        end
    end
    return array
end

function event_handler(name, handler)
    
end

function ip_to_country_code(ip)
    return ""
end
