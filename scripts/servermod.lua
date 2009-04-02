
dofile("./scripts/logging.lua")

function server.log_status(msg)
    print(msg)
end

function onText(cn,text)
    if server.process_player_command(cn,text) then return -1 end
end

function onShutdown()
    print("Server shutting down.")
end

server.event_handler("text",onText)
server.event_handler("shutdown",onShutdown)

server.script_socket_server(7894)

server.loadbanlist("conf/bans")
server.load_geoip_database("share/GeoIP.dat")
