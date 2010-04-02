--[[

	A player command to list players with frags, deaths and accuracies

	Copyright (C) 2009 Thomas

]]


return function(cn)

	for p in server.gclients() do

		server.player_msg(cn, "Player: " .. p:displayname() .. " Frags: " .. p:frags() .. " Deaths: " .. p:deaths() .. " Acc: " .. p:accuracy() .. "%")
	end

end
