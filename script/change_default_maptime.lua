server.changetime(server.default_maptime)

server.event_handler("mapchange",function(map,mode)
    server.changetime(server.default_maptime)
end)
