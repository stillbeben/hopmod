if server.exp_teamkill_limiter == 0 then
    local teamkill_limit = server.teamkill_limit
    
    function onTeamkill(actor, victim)
	if not server.player_var(actor,"shown_teamkill_warning") then
    	    if server.teamkill_showlimit == 1 then
        	server.player_msg(actor,"This " .. orange("server will not tolerate") .. " more than " .. tklimiter.tklimit .. orange(" team kills") .. " per game.")
    	    else
    		server.player_msg(actor,"This " .. orange("server enforces a team kill limit") .. ", and so you need to " .. orange("play more carefully") .. ". You have been warned.")
    	    end
    	    server.player_var(actor,"shown_teamkill_warning",true)
	end
	if server.teamkill_show_public == 1 then
	    server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(actor)) .. "(" .. magenta(actor) .. ") " .. orange("killed a teammate."))
	end
	if server.player_teamkills(actor) > teamkill_limit then
    	    server.kick(actor,server.teamkill_bantime,"server","teamkilling")
	end
    end
    
    server.event_handler("teamkill", onTeamkill)
    
else
    local tklimiter = {}
    
    tklimiter.tklimit = server.teamkill_limit
    tklimiter.tklimit_1 = round(tklimiter.tklimit / 4)
    tklimiter.tklimit_2 = round(tklimiter.tklimit / 2)
    tklimiter.tklimit_3 = tklimiter.tklimit_1 * 3
    
    server.event_handler("teamkill",function(actor,victim)
	if not server.player_var(actor,"shown_teamkill_warning") then
    	    if server.teamkill_showlimit == 1 then
        	server.player_msg(actor,"This " .. orange("server will not tolerate") .. " more than " .. (tklimiter.tklimit - (server.player_pvars(actor).tklimiter_last_tks or 0)) .. orange(" team kills") .. " per game.")
    	    else
    		server.player_msg(actor,"This " .. orange("server enforces a team kill limit") .. ", and so you need to " .. orange("play more carefully") .. ". You have been warned.")
    	    end
    	    server.player_var(actor,"shown_teamkill_warning",true)
	end
	if server.teamkill_show_public == 1 then
	    server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(actor)) .. "(" .. magenta(actor) .. ") " .. orange("killed a teammate."))
	end
	if (server.player_teamkills(actor) + (server.player_pvars(actor).tklimiter_last_tks or 0)) > tklimiter.tklimit then
    	    server.kick(actor,server.teamkill_bantime,"server","teamkilling")
	end
    end)
    
    function tklimiter.check_tks(cn)
	local current_tks = server.player_teamkills(cn) + round((server.player_pvars(cn).tklimiter_last_tks or 0) / 3)
	if current_tks >= tklimiter.tklimit_3 then
	    server.player_pvars(cn).tklimiter_last_tks = tklimiter.tklimit_2
	elseif current_tks >= tklimiter.tklimit_2 then
	    server.player_pvars(cn).tklimiter_last_tks = tklimiter.tklimit_1
	elseif current_tks >= tklimiter.tklimit_1 then
	    server.player_pvars(cn).tklimiter_last_tks = 1			-- fix.me?!
        else
    	    server.player_pvars(cn).tklimiter_last_tks = 0
        end
    end
    
    server.event_handler("finishedgame",function()
	for j,cn in ipairs(server.spectators()) do
	    tklimiter.check_tks(cn)
        end
        for p in server.gplayers() do
    	    tklimiter.check_tks(p.cn)
        end
    end)
    
end
