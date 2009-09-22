-- [[ based on a player command written by Thomas ]] --

return function(cn, n)
    
	if not n then
		server.player_msg(cn,red("#maxclients <size>"))
		return
--		return false, "missing argument"
	end

	n = tonumber(n)

	if n >= server.playercount and n <= 128 then
		server.maxplayers = n
	end
    
end
