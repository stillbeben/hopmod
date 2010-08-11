--[[
    A script to move inactive players to spectators
]]

local interval_time = server.inactivity_check_time
if interval_time <= 0 then
    interval_time = 60000
end
local inactive_time = round((server.inactivity_time / 1000), 0)
local death_only = server.inactivity_death_only == 1

local is_unload


local function unset_vars(cn)

    server.player_vars(cn).inactivity_pos_x = nil
    server.player_vars(cn).inactivity_pos_y = nil
    server.player_vars(cn).inactivity_pos_z = nil
    server.player_vars(cn).inactivity_pos_time = nil
end

local function unset_vars_all()

    for p in server.gplayers() do
	unset_vars(p.cn)
    end
end


server.event_handler("disconnect", unset_vars)

server.event_handler("mapchange", unset_vars_all)

server.event_handler("spectator", unset_vars)

server.interval(interval_time, function()

    if is_unload
    then
	return -1
    end
    
    if (server.mastermode == 0) or (server.mastermode == 1)
    then
	for p in server.gplayers()
	do
	    local spec_player
	    
	    local last_x, last_y, last_z = p:vars().inactivity_pos_x, p:vars().inactivity_pos_y, p:vars().inactivity_pos_z
	    local con_time = p:connection_time()
	    
	    if not (last_x and last_y and last_z)
	    then
	        p:vars().inactivity_pos_x, p:vars().inactivity_pos_y, p:vars().inactivity_pos_z = p:pos()
	        p:vars().inactivity_pos_time = con_time
	    else
	        local x, y, z = p:pos()
		
		if (last_x == x) and (last_y == y) and (last_z == z)
		then
		    local last_time = p:vars().inactivity_pos_time
		    
		    if con_time - last_time >= inactive_time
		    then
			spec_player = true
			
			if death_only and not (p:status_code() == server.DEAD)
			then
			    spec_player = nil
			end
			
			unset_vars(p.cn)
		    end
		else
		    p:vars().inactivity_pos_x, p:vars().inactivity_pos_y, p:vars().inactivity_pos_z = x, y, z
	    	    p:vars().inactivity_pos_time = con_time
		end
	    end
	    
	    if spec_player
	    then
		p:msg("Server moved you to spectators, because you seem to be inactive - type '/spectator 0' to rejoin the game.")
		p:spec()
		server.log("Server moved " .. p:name() .. " to spectator, because of inactivity.")
	    end
	end
    end
end)


local function unload()

    is_unload = true
    
    unset_vars_all()
end


return {unload = unload}
