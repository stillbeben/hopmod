local min_played_time = 180
local min_frags = 15


local events = {}


events.finishedgame = server.event_handler_object("finishedgame",function()

	for p in server.aplayers() do
		local acc = p:accuracy()
		local time = p:timeplayed()
		local frags = p:frags() + p:teamkills() + p:suicides()

		if (frags > min_frags) and (time > min_played_time) and (acc > server.cd_accuracy_limit) then
			server.log("WARNING: " .. p:name() .. "(" .. p.cn .. ") had an accuracy of: " .. acc .. "%. [frags/deaths: " .. frags .. "/" .. p:deaths() .. " | ip: " .. p:ip() .. "]")
		end
	end

end)


local function disable()

	events = nil

end


return {unload = disable}
