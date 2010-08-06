local logfile = io.open("log/server.log","a+")

function server.log(msg)

    assert(msg ~= nil)
    logfile:write(os.date("[%a %d %b %X] ",os.time()))
    logfile:write(msg)
    logfile:write("\n")
    logfile:flush()
end

server.event_handler("shutdown", function(shutdown_type)

    logfile:close()
end)
