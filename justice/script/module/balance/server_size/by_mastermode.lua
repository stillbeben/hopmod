local max_players = server.maxplayers
local total_max_players = server.resize_server_mastermode_size
local resize_mastermode = server.resize_server_mastermode_using_mastermode

local function resize_mastermodechange(mmode)

	if mmode == resize_mastermode then
		server.maxplayers = total_max_players
	else
		local pcount = server.playercount

		if pcount > max_players then
			server.maxplayers = pcount
		else
			server.maxplayers = max_players
		end
	end
end

server.event_handler("setmastermode", function(cn, old, new)
	server.sleep(500, function()
		resize_mastermodechange(server.mastermode)
	end)
end)

server.event_handler("disconnect", function(cn)
	server.sleep(500, function()
		resize_mastermodechange(server.mastermode)
	end)
end)

server.event_handler("mapchange", function(map, mode)
	server.sleep(500, function()
		resize_mastermodechange(server.mastermode)
	end)
end)

server.event_handler("started", function()
	server.sleep(500, function()
		resize_mastermodechange(server.mastermode)
	end)
end)

local function unload()
	server.maxplayers = max_players
end

return {unload = unload}
