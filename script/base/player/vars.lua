require "Json"
require "net"

local VARS_FILE = "log/player_vars"
local MINIMUM_UPDATE_INTERVAL = 1000 * 60 * 60

local variablesByIp = {}
local variablesByIpIndex = net.ipmask_table()
local variablesById = {}

local lastsave = 0

local function checkValue(value)
    if type(value) == "function" or type(value) == "userdata" then error("cannot store value of a " .. type(value) .. " type") end
end

local function saveVars()

    local file = io.open(VARS_FILE, "w")
    if not file then
        server.log_error("Unable to save player vars")
    end
    file:write(Json.Encode(variablesByIp))
    file:close()
end

local function loadVars()

    local file = io.open(VARS_FILE)
    if not file then return end
    variablesByIp = Json.Decode(file:read("*a")) or {}
    file:close()
    
    for key, value in pairs(variablesByIp) do
        variablesByIpIndex[key] = value
    end
end

function server.player_vars(cn)
    local id = server.player_id(cn)
    if id == -1 then error("invalid cn") end
    local vars = variablesById[id]
    return vars
end

function server.set_ip_var(ipmask, name, value)
    
    local matches = variablesByIpIndex[ipmask]
    local vars = matches[#matches]
    
    if not vars then
        vars = {}
        variablesByIp[ipmask] = vars
        variablesByIpIndex[ipmask] = vars
    end
    
    vars[name] = value
    
    for _, cn in ipairs(server.clients()) do
        if net.ipmask(ipmask) == net.ipmask(server.player_iplong(cn)) then
            local id = server.player_id(cn)
            variablesById[id][name] = value
        end
    end
end

server.event_handler("connect", function(cn)

    local id = server.player_id(cn)
    variablesById[id] = variablesById[id] or {}
    
    local matches = variablesByIpIndex[net.ipmask(server.player_iplong(cn))]
    local vars = variablesById[id]
    for _, match in ipairs(matches) do
        for key, value in pairs(match) do
            vars[key] = vars[key] or value
        end
    end
end)

server.event_handler("renaming", function(cn, futureId)
    local currentId = server.player_id(cn)
    variablesById[futureId] = variablesById[currentId]
    variablesById[currentId] = nil
end)

server.event_handler("disconnect", function()
    if server.playercount == 0 and server.uptime - lastsave > MINIMUM_UPDATE_INTERVAL then
        saveVars()
        lastsave = server.uptime
    end
end)

server.event_handler("started", loadVars)
server.event_handler("shutdown", saveVars)
