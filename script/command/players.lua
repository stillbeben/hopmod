-- list players + player stats
-- [[ based on a player command written by Thomas ]] --


return function(cn)

	for p in server.gplayers() do
		local acc = 0
		local shots = p:shots()
		if not (shots == 0) then
			acc = round(p:hits()/p:shots(),2)
		end

		server.player_msg(cn, "Name: " .. p:name() .. " Frags: " .. p:frags() .. " Deaths: " .. p:deaths() .. " Acc: " .. acc .. "%")
	end

end
