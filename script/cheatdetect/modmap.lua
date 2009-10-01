local events = {}


-- helper
--
-- send message and set person to spectator
local function stresser(cn)

	server.player_msg(cn,"(" .. green("Info") .. ")  You have a " .. red("modified map"))
	server.player_msg(cn,"(" .. green("Info") .. ")  " .. orange("You cannot play on this map!") .. " Please, wait for the next map")
	server.player_msg(cn,"(" .. green("Info") .. ")  and " .. orange("re-download the game") .. " from " .. yellow("www.sauerbraten.org"))
	server.spec(cn)

end


-- events
--
-- safe player, when he has a modmap [and mode is not coop and map is known]
events.mapcrcfail = server.event_handler_object("mapcrcfail",function(cn)

	if ( server.playercount > 2 ) and ( not (server.gamemode == "coop edit") ) and ( server.is_known_map(server.map) ) then
		server.player_vars(cn).modmap = true
		stresser(cn)
		server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(cn)) .. " has a " .. red("modified map"))
	end

end)

-- check safed player, when he tries to leave spectator
events.spectator = server.event_handler_object("spectator",function(cn,joined)

	if ( joined == 0 ) and ( server.player_vars(cn).modmap ) then
		stresser(cn)
	end

end)


local function disable()

	events = {}

end


return {unload = disable}
