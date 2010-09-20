local max_players = server.maxplayers

local total_max_players	= server.resize_server_mastermode_size
local resize_mastermode	= server.resize_server_mastermode_using_mastermode


local function resize()

    server.sleep(500, function()
    
	if server.mastermode == resize_mastermode
	then
	    server.maxplayers = total_max_players
	else
	    server.maxplayers = max_players
	end
    end)
end


server.event_handler("setmastermode", resize)

server.event_handler("mapchange", resize)

server.event_handler("disconnect", resize)

server.event_handler("started", resize)


return {unload = function()

    server.maxplayers = max_players
end}
