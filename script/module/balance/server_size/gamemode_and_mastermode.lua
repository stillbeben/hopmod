local max_players = server.maxplayers
local total_max_players = server.resize_server_mastermode_size
local resize_mastermode = server.resize_server_mastermode_using_mastermode

local size_gamemode = {}
size_gamemode["regen capture"] = server.resize_server_gamemode_size_regen_capture or max_players
size_gamemode["capture"] = server.resize_server_gamemode_size_capture or max_players
size_gamemode["insta ctf"] = server.resize_server_gamemode_size_insta_ctf or max_players
size_gamemode["ctf"] = server.resize_server_gamemode_size_ctf or max_players
size_gamemode["insta protect"] = server.resize_server_gamemode_size_insta_protect or max_players
size_gamemode["protect"] = server.resize_server_gamemode_size_protect or max_players
size_gamemode["teamplay"] = server.resize_server_gamemode_size_teamplay or max_players
size_gamemode["efficiency team"] = server.resize_server_gamemode_size_efficiency_team or max_players
size_gamemode["tactics team"] = server.resize_server_gamemode_size_tactics_team or max_players
size_gamemode["instagib team"] = server.resize_server_gamemode_size_instagib_team or max_players
size_gamemode["ffa"] = server.resize_server_gamemode_size_ffa or max_players
size_gamemode["efficiency"] = server.resize_server_gamemode_size_efficiency or max_players
size_gamemode["tactics"] = server.resize_server_gamemode_size_tactics or max_players
size_gamemode["instagib"] = server.resize_server_gamemode_size_instagib or max_players
size_gamemode["coop edit"] = server.resize_server_gamemode_size_coop_edit or max_players

local function resize_modechange(mode, mmode)

	if mmode == resize_mastermode then
		server.maxplayers = total_max_players
	else
		local pcount = server.playercount

		if pcount > size_gamemode[mode] then
			server.maxplayers = pcount
		else
			server.maxplayers = size_gamemode[mode]
		end
	end
end

server.event_handler("setmastermode", function(cn, old, new)
	server.sleep(500, function()
		resize_modechange(server.gamemode, new)
	end)
end)

event.mapchange = server.event_handler("mapchange", function(map, mode)
	server.sleep(500, function()
		resize_modechange(mode, server.mastermode)
	end)
end)

event.disconnect = server.event_handler("disconnect", function(cn, reason)
	server.sleep(500, function()
		resize_modechange(server.gamemode, server.mastermode)
	end)
end)

event.started = server.event_handler("started", function()
	server.sleep(500, function()
		resize_modechange(server.gamemode, server.mastermode)
	end)
end)

local function unload()
	server.maxplayers = max_players
end

return {unload = unload}
