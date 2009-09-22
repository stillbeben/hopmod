-- [[ based on a player command written by Thomas ]] --

return function(cn)

    for p in server.gplayers() do
		server.player_msg(cn, "Player: " .. green(p:name()) .. " Country: " ..  orange(server.ip_to_country(p:ip())))
	end

end
