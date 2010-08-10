--[[
    suddendeath mode: game will not end while there is a tie
    so far there is a tie when timelimit is reached, the next score will clearify
    
    command function: suddendeath(enable)
    to enable (enable = true)/ disable (enable = nil)
    to enable has no effect on the current map, when maptime < 1 min
]]

local default_enabled = server.suddendeath_enabled_by_default == 1

local is_active
local is_tie
local is_checked
local is_new_map

-- ffa: frag, sucide events
-- capture: no event
-- ctf: scoreflag event
-- protect: scoreflag, resetflag events
local map_mode = {
    ["ffa"] = "ffa",
    ["teamplay"] = "ffa",
    ["tactics"] = "ffa",
    ["tactics team"] = "ffa",
    ["efficiency"] = "ffa",
    ["efficiency team"] = "ffa",
    ["instagib"] = "ffa",
    ["instagib team"] = "ffa",
    ["ctf"] = "ctf",
    ["insta ctf"] = "ctf",
    ["efficiency ctf"] = "ctf",
    ["hold"] = "ctf",
    ["insta hold"] = "ctf",
    ["efficiency hold"] = "ctf",
    ["protect"] = "protect",
    ["insta protect"] = "protect",
    ["efficiency protect"] = "protect",
    ["capture"] = "capture",
    ["regen capture"] = "capture"
}
local mode = {}

local msg_start = "Suddendeath - next score wins!"

local is_unload


local function unset_vars()

    is_tie = nil
    mode = {}
end

local function stop_game()

    unset_vars()
    server.changetime(0)
end

local function event_mode_ffa()

    if is_active and mode.ffa and is_tie
    then
	stop_game()
    end
end


server.suddendeath = function(enable)

    unset_vars()
    
    if not enable
    then
	is_active = nil
    else
	is_active = true
    end
end


server.interval(1000, function()		-- no event

    if is_unload
    then
	return -1
    end
    
    if is_active and mode.capture and is_tie
    then
	for _, team in pairs(server.teams())
	do
	    local score = server.team_score(team)
	    
	    for _, t in pairs(server.teams())
	    do
		if not (team == t) and (score == server.team_score(t))
		then
		    stop_game()
		    return
		end
	    end
	end
    end    
end)

server.event_handler("connect", function()

    if server.playercount == 1
    then
        if default_enabled
        then
            server.suddendeath(true)
        else
            server.suddendeath()
        end
    end
end)

server.event_handler("frag", event_mode_ffa)

server.event_handler("suicide", event_mode_ffa)

server.event_handler("scoreflag", function()

    if is_active and (mode.ctf or mode.protect) and is_tie
    then
	stop_game()
    end
end)

server.event_handler("resetflag", function()

    if is_active and mode.protect and is_tie
    then
	stop_game()
    end
end)

server.event_handler("timeupdate", function(mins)

    if (mins == 1) and is_active and not gamemodeinfo.edit
    then
	is_new_map = nil
	
	server.sleep(58000, function()
	
	    if is_active and not is_new_map
	    then
		if not is_checked
		then
		    is_checked = true
		    
		    is_tie = nil
		    
		    if gamemodeinfo.teams
		    then
			for _, team in pairs(server.teams())
			do
			    local score = server.team_score(team)
			    
			    for _, t in ipairs(server.teams())
			    do
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
			mode[map_mode[server.gamemode]] = true
			
			server.msg(msg_start)
		    end
		end
		
		if is_tie
		then
		    server.changetime(60000)
		end
	    end
	end)
    end
end)

server.event_handler("mapchange", function()

    is_new_map = true
    is_checked = nil
end)


return {unload = function()

    is_unload = true
    
    unset_vars()
    
    is_active, is_new_map, is_checked = nil, nil, nil
    
    server.unref("suddendeath")
end}
