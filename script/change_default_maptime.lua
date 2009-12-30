
local map_time = server.default_maptime

local event = {}


event.mapchange = server.event_handler_object("mapchange", function(map, mode)

	server.changetime(map_time)

end)

event.started = server.event_handler_object("started", function()

	server.changetime(map_time)

end)


local function unload()

	event = {}

end


return {unload = unload}
