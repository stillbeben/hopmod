local min_played_time = 180

server.event_handler("finishedgame",function()
    for a,b in ipairs(server.players()) do
	local acc = server.player_accuracy(b)
	local time = server.player_timeplayed(b)
	if (time > min_played_time) and (acc > server.cd_accuracy_limit) then
	    server.log("WARNING: " .. server.player_name(b) .. "(" .. b .. ") had an accuracy of: " .. acc .. "%. [frags/deaths: " .. ((server.player_frags(b) + server.player_teamkills(b)) + server.player_suicides(b)) .. "/" .. server.player_deaths(b) .. " | ip: " .. server.player_ip(b) .. "]")
	end
    end
end)
