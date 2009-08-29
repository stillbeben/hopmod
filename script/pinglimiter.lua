local ping_limiter_is_intermission = 0

server.event_handler("intermission",function()
    ping_limiter_is_intermission = 1
end)

server.event_handler("mapchange",function(map,mode)
    ping_limiter_is_intermission = 0
end)

server.interval(server.ping_limiter_tick,function()
    if tonumber(server.playercount) > 2 then
        for a,b in ipairs(server.players()) do
    	    local warnings = (server.player_pvars(b).ping_warnings or 0)
            if (ping_limiter_is_intermission == 0) and (server.player_lag(b) > server.lag_limit) and (server.player_ping(b) > server.ping_limit) then
		warnings = warnings + 1
		if warnings > server.ping_limit_warnings then
		    server.log("Net stats for " .. server.player_name(b) .. ": ping " .. server.player_ping(b) .. " lag " .. server.player_lag(b))
            	    server.kick(b,1,"server","high ping")
            	    server.player_pvars(b).ping_warnings = 0
            	else
            	    server.player_msg(b,green(server.player_name(b)) .. ", your " .. red("ping is too high") .. " for this server - it needs to be " .. orange("below " .. server.ping_limit .. " ms."))
            	    server.player_pvars(b).ping_warnings = warnings
            	end
            end
        end
    end
end)
