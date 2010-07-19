local default_mastermode = server.default_mastermode

local event = {}


event.setmaster = server.event_handler_object("setmaster", function(cn)

	if server.mastermode < default_mastermode then
		server.mastermode = default_mastermode
	end

end)

event.disconnect = server.event_handler_object("disconnect", function(cn, reason)

	if server.playercount == 0 then
		server.mastermode = default_mastermode
	elseif server.mastermode < default_mastermode then
		server.mastermode = default_mastermode
	end

end)

event.started = server.event_handler_object("started", function()

	server.mastermode = default_mastermode

end)


local function unload()

	event = {}
	server.mastermode = 0

end


return {unload = unload}
