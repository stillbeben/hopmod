local filename = "server"
if (server.is_authserver or 0) == 1
then
    filename = "authserver"
end

local logfile = io.open("log/" .. filename .. ".log","a+")

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
