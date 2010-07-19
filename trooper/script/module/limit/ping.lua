local in_intermission = false
local still_loaded = true
local unloadEventHandlers
local ping_limit = server.ping_limit
local lag_limit = server.lag_limit

local function loadEventHandlers()
    
    local intermission = server.event_handler("intermission", function() in_intermission = true end)
    local mapchange = server.event_handler("mapchange", function() in_intermission = false end)
    
    unloadEventHandlers = function()
    
        server.cancel_handler(intermission)
        server.cancel_handler(mapchange)
        
        still_loaded = false
    end
end

server.interval(server.ping_limiter_tick, function()

    if not still_loaded then return -1 end
    if server.playercount < 3 then return end
    
    for player in server.gplayers() do
        
        local warnings = player:pvars().ping_warnings or 0
        
        if in_intermission == false and player:lag() > lag_limit and player:ping() > ping_limit then
            
            warnings = warnings + 1
            
            if warnings > server.ping_limit_warnings then
            
                server.log("Net stats for " .. player:name() .. ": ping " .. player:ping() .. " lag " .. player:lag())
                
                player:kick(1, "server", "high ping")
                player:pvars().ping_warnings = 0
                
            else
                
                player:msg(red(string.format("Your ping is too high! The server expects pings to be below %i.", ping_limit)))
                player:pvars().ping_warnings = warnings
            end
        end
    end
end)

loadEventHandlers()

return {unload = unloadEventHandlers}
