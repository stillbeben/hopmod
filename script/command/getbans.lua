return function(cn)

	for index, ip in ipairs(server.bans()) do
		server.player_msg(cn, "IP: " .. ip)
	end

end
