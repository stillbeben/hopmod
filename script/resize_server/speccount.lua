local event = {}

local max_players = server.maxplayers

local total_max_players = server.resize_server_speccount_max_active_players


local function calcOutput(players, specs)

	return players ^ 2 - players + (players * specs)

end


local output_limit = calcOutput(max_players, 0)
local output_max_limit = calcOutput(total_max_players, 0)


local function readjustCapacity()

	local sc = server.speccount
	local adminslots = server.reservedslots_occupied
	local pc = server.playercount - sc - adminslots
	local extra = -1

	while calcOutput(pc + (extra + 1), sc) <= output_limit do
		extra = extra + 1
	end

	if gamemodeinfo.teams then
		extra = extra - ((pc + extra)%2)
	end

	server.maxplayers = pc + extra + sc + adminslots

end


local function isOverCapacity()

	return calcOutput(server.playercount, server.speccount) > output_max_limit

end


event.spectator = server.event_handler_object("spectator", function(cn, value)

	if value == 0 and isOverCapacity() then

		server.sleep(1, function()

			server.spec(cn)
			server.player_msg(cn, red("Sorry, there are too many players in the game at the moment, you'll have to wait until someone leaves the game."))

		end)

	end

	readjustCapacity()

end)


event.disconnect = server.event_handler_object("disconnect", function(cn, reason)

	readjustCapacity()

end)


local function unload()

	event = {}

	server.maxplayers = max_players

end


return {unload = unload}
