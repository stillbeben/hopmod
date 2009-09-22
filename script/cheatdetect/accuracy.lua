local min_played_time = 180
local min_frags = 15


server.event_handler("finishedgame",function()

	for p in server.gplayers() do
		local acc = server.player_acc(p.cn)
		local time = p:timeplayed()
		local frags = p:frags()

		if (frags > min_frags) and (time > min_played_time) and (acc > server.cd_accuracy_limit) then
			server.log("WARNING: " .. p:name() .. "(" .. p.cn .. ") had an accuracy of: " .. acc .. "%. [frags/deaths: " .. ((frags + p:teamkills()) + p:suicides()) .. "/" .. p:deaths() .. " | ip: " .. p:ip() .. "]")
		end
	end

end)
