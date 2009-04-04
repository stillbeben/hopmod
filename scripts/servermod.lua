dofile("./scripts/serverlib.lua")
dofile("./scripts/logging.lua")

function onText(cn,text)
    
    local block = 0
    
    if server.process_player_command(cn,text) then block = -1 end
    
    if block == 0 and server.player_pvar(cn,"mute") then
        server.player_msg(cn, red() .. "Your chat messages are being blocked.")
        block = -1
    end
    
    return block
end

server.event_handler("text",onText)
server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

server.script_socket_server(7894)

server.loadbanlist("conf/bans")
server.load_geoip_database("share/GeoIP.dat")

server.log_status("-> Successfully loaded Hopmod 4")
