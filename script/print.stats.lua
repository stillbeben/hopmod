local function statusprinter()
    server.printserverstatus("log/server.status","w+")
end

statusprinter()

server.event_handler("mapchange",function(map,mode)
    statusprinter()
end)

server.event_handler("active",function(cn)
    statusprinter()
end)

server.event_handler("disconnect",function(cn)
    statusprinter()
end)

server.event_handler("spectator",function(cn,joined)
    statusprinter()
end)

server.event_handler("setmastermode",function(old,new)
    server.sleep(1000,function()
	statusprinter()
    end)
end)

server.event_handler("shutdown",function()
    local file = io.open("log/server.status","w+")
    file:write("\n")
    file:flush()
    file:close()
end)
