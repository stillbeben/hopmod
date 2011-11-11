
local usage =  "#motd \"<text>\""

return function(cn, ip)

	if not ip then
		return false, usage
	end
    
	if tonumber(server.unsetban(ip)) == 1 then
		server.player_msg(cn, "done, check with #banlist")
	else
		server.player_msg(cn, "no matching ban found")
	end

end
