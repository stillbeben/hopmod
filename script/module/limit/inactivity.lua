--[[
    A script to move inactive players to spectators
]]

local interval_time = server.spec_inactives_check_time
if interval_time <= 0 then
    interval_time = 60000
end
local inactive_time = round((server.spec_inactives_time / 1000), 0)
local death_only = server.spec_inactives_death_only == 1

local is_unload


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
	    
	    local last_x, last_y, last_z = p:vars().spec_inactives_pos_x, p:vars().spec_inactives_pos_y, p:vars().spec_inactives_pos_z
	    
	    if not (last_x and last_y and last_z)
	    then
	        p:vars().spec_inactives_pos_x, p:vars().spec_inactives_pos_y, p:vars().spec_inactives_pos_z = p:pos()
	        p:vars().spec_inactives_pos_time = p:connection_time()
	    else
	        local x, y, z = p:pos()
		
		if (last_x == x) and (last_y == y) and (last_z == z)
		then
		    local last_time = p:vars().spec_inactives_pos_time
		    
		    if p:connection_time() - last_time >= inactive_time
		    then
			spec_player = true
			
			if death_only and not (p:status_code() == server.DEAD)
			then
			    spec_player = nil
			end
			
			p:vars().spec_inactives_pos_x, p:vars().spec_inactives_pos_y, p:vars().spec_inactives_pos_z = nil, nil, nil
			p:vars().spec_inactives_pos_time = nil
		    end
		else
		    p:vars().spec_inactives_pos_x, p:vars().spec_inactives_pos_y, p:vars().spec_inactives_pos_z = nil, nil, nil
		    p:vars().spec_inactives_pos_time = nil
		end
	    end
	    
	    if spec_player
	    then
		p:spec()
		p:msg("Server moved you to spectators, because you seem to be inactive - type " .. yellow("/spectator 0") .. " to rejoin the game.")
	    end
	end
    end
end)


local function unload()

    is_unload = true
    
    for p in server.gclients() do
	p:vars().spec_inactives_deathtime = nil
	p:vars().spec_inactives_pos_x, p:vars().spec_inactives_pos_y, p:vars().spec_inactives_pos_z = nil, nil, nil
	p:vars().spec_inactives_pos_time = nil
    end
end


return {unload = unload}
