
local callbacks = {}

function server.map_loaded(callback)
    table.insert(callbacks, callback)
end

local function callback_on_ping_activity(cn, callback)
    local sid = server.player_sessionid(cn)
    
    -- NOTE: client can send ping packets before loading map
    server.interval(2000, function() 
    
        if sid ~= server.player_sessionid(cn) then
            return -1
        end
        
        if server.player_ping_update(cn) > server.uptime - 2000 then 
            callback(cn)
            return -1
        end
    end)
end

server.event_handler("active", function(cn)
    for _, callback in ipairs(callbacks) do
        callback(cn)
    end
end)

local function callback_for_spectators(cn)
    for _, callback in ipairs(callbacks) do
        if server.player_status_code(cn) == server.SPECTATOR then
            callback_on_ping_activity(cn, callback)
        end
    end
end

server.event_handler("connect", callback_for_spectators)

server.event_handler("mapchange", function() 
    for _,cn in ipairs(server.spectators()) do 
        callback_for_spectators(cn) 
    end
end)
