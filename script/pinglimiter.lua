server.interval(server.ping_limiter_tick,function()
    if tonumber(server.playercount) > 2 then
        for a,b in ipairs(server.players()) do
    	    local warnings = nil
            if not server.player_pvars(b).ping_warnings then
        	warnings = 0
    	    else
        	warnings = server.player_pvars(b).ping_warnings
    	    end
            if server.player_lag(b) > server.lag_limit then
		warnings = warnings + 1
		if warnings > server.ping_limit_warnings then
		    server.log("Net stats for " .. server.player_name(b) .. ": ping " .. server.player_ping(b) .. " lag " .. server.player_lag(b))
		    local reason = "high pj"
        	    if server.player_ping(b) > server.ping_limit then
			reason = "high ping"
                    end
            	    server.kick(b,0,"server",reason)
            	    server.player_pvars(b).ping_warnings = 0
            	else
            	    local message = ", your " .. red("pj is too high") .. " for this server - it needs to be " .. orange("below " .. server.lag_limit)
            	    if server.player_ping(b) > server.ping_limit then
			message = ", your " .. red("ping is too high") .. " for this server - it needs to be " .. orange("below " .. server.ping_limit)
                    end
            	    server.player_msg(b,green(server.player_name(b)) .. message .. " ms.")
            	    server.player_pvars(b).ping_warnings = warnings
            	end
            end
        end
    end
end)
