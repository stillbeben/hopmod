function server.playercmd_setmaster(setmaster_cn)
    if server.player_priv_code(tonumber(setmaster_cn)) > 1 then
	server.unsetmaster(setmaster_cn)
    else
	server.setmaster(setmaster_cn)
	server.msg(server.player_name(setmaster_cn) .. " claimed master")
    end
end
