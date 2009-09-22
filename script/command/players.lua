-- list players + player stats
-- [[ based on a player command written by Thomas ]] --


return function(cn)

	for p in server.gplayers() do
		server.player_msg(cn, "Name: " .. p:name() .. " Frags: " .. p:frags() .. " Deaths: " .. p:deaths() .. " Acc: " .. server.player_acc(p.cn) .. "%")
	end

end
