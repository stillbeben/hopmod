return function(cn, ip)
	server.unsetban(ip)
	server.player_msg(cn, "done, check with #getbans")
end