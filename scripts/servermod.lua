
function server.log_status(msg)
    print(msg)
end

function server.kick(cn, bantime, admin, reason)
    server.player_kick(cn, bantime or 14400, admin or "server", reason or "")
end

function onText(cn,text)
    if server.process_player_command(cn,text) then return -1 end
end

function onShutdown()
    print("Server shutting down.")
end

server.event_handler("text",onText)
server.event_handler("shutdown",onShutdown)

server.load_geoip_database("share/GeoIP.dat")
