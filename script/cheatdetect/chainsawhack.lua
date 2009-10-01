local events = {}

events.frag = server.event_handler_object("frag",function(tcn,acn)

	local a_pos_x,a_pos_y,a_pos_z = server.player_pos(acn)
	local t_pos_x,t_pos_y,t_pos_z = server.player_pos(tcn)
	local weapon = server.player_gun(acn)

	if weapon == 0 then
		if (math.abs(a_pos_x - t_pos_x) > 24) or (math.abs(a_pos_y - t_pos_y) > 24) or (math.abs(a_pos_z - t_pos_z) > 24) then
			server.log("WARNING: " .. server.player_name(acn) .. "(" .. acn .. ") uses a chainsaw hack.  [pj: " .. server.player_lag(acn) .. " | ping: " .. server.player_ping(acn) .. " | ip: " .. server.player_ip(acn) .. "]")
		end
	end

end)


local function disable()

	events = nil

end


return {unload = disable}
