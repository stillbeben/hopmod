--[[
    like suddendeath: game will not end, while there is no clear ranking order
    increasess game time by one minute
    
    command function: no_ties(enable)
	to enable (enable = true)/ disable (enable = nil)
	no_ties(true) has no effect on the current map, when maptime < 1
]]

local default_enabled = server.no_ties_enabled_by_default == 1

local is_active
local is_new_map

local msg_overtime = "One minute overtime!"


function server.no_ties(enable)

    if not enable
    then
	is_active = nil
    else
	is_active = true
    end
end


server.event_handler("connect", function()

    if server.playercount == 1
    then
	if default_enabled
	then
    	    server.no_ties(true)
	else
    	    server.no_ties()
	end
    end
end)

server.event_handler("timeupdate", function(mins)

    if (mins == 1) and is_active and not gamemodeinfo.edit
    then
	is_new_map = nil
	
	server.sleep(58000, function()
	
	    if is_active and not is_new_map
	    then
		local is_tie
		
		if gamemodeinfo.teams
		then
		    for _, team in pairs(server.teams())
		    do
			local score = server.team_score(team)
			
			for _, t in pairs(server.teams()) do
	    		    if not (team == t) and (score == server.team_score(t))
	    		    then
				is_tie = true
				break
			    end
			end
			
			if is_tie
			then
			    break
			end
		    end
		else
    		    for player in server.gplayers()
    		    do
    			local cn = player.cn
    			local score = player:score()
    			
			for p in server.gplayers()
			do
	    		    if not (cn == p.cn) and (score == p:score())
	    		    then
				is_tie = true
				break
			    end
			end
			
			if is_tie
			then
			    break
			end
		    end
		end
		
		if is_tie
		then
		    server.msg(msg_overtime)
		    server.changetime(60000)
		end
	    end
	end)
    end
end)

server.event_handler("mapchange", function()

    is_new_map = true
end)


return {unload = function()

    is_active, is_new_map = nil, nil
    
    server.unref("no_ties")
end}
