server.event_handler("finishedgame",function()
    for a,b in ipairs(server.players()) do
	local acc = server.player_accuracy(b)
	if acc > server.cd_accuracy_limit then
	    server.log("WARNING: " .. server.player_name(b) .. " had an accuracy of: " .. acc .. "%. [cn: " .. b .. " | ip: " .. server.player_ip(b) .. "]")
	end
    end
end)
