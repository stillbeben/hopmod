--[[

	A player command to list all players and their country

	Copyright (C) 2009 Thomas

]]

require "geoip"

return function(cn)

	for p in server.gclients() do
		server.player_msg(cn, "Player: " .. green(p:displayname()) .. " Country: " ..  orange(geoip.ip_to_country(p:ip())))
	end
end
