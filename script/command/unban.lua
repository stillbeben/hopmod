return function(cn, ip)

	if tonumber(server.unsetban(ip)) == 1 then
		server.player_msg(cn, "done, check with #getbans")
	else
		server.player_msg(cn, "no matching ban found")
	end

end
