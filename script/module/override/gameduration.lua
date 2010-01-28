local map_time = server.default_maptime
local event = {}

local function changeTime()
    server.changetime(map_time)
end

event.mapchange = server.event_handler_object("mapchange", changeTime)
event.started = server.event_handler_object("started", changeTime)

local function unload()
	event = {}
end

return {unload = unload}
