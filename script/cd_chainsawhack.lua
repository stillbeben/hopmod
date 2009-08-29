local function cd_chainsaw_hack_warning(cd_chainsaw_hack_warning_cn)
    server.log("WARNING: " .. server.player_name(cd_chainsaw_hack_warning_cn) .. "(" .. cd_chainsaw_hack_warning_cn .. ") uses a chainsaw hack.  [ip: " .. server.player_ip(cd_chainsaw_hack_warning_cn) .. "]")
end

server.event_handler("frag",function(tcn,acn)
    local weapon = server.player_gun(acn)
    local a_pos_x,a_pos_y,a_pos_z = server.player_pos(acn)
    local t_pos_x,t_pos_y,t_pos_z = server.player_pos(tcn)
    
    if weapon == 0 then
	if math.abs(a_pos_x - t_pos_x) > 24 then
	    cd_chainsaw_hack_warning(acn)
	elseif math.abs(a_pos_y - t_pos_y) > 24 then
	    cd_chainsaw_hack_warning(acn)
	elseif math.abs(a_pos_z - t_pos_z) > 24 then
	    cd_chainsaw_hack_warning(acn)
	end
    end
end)
