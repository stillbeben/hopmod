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
server.event_handler("sayteam", onText)
server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

server.script_socket_server(server.script_socket_port)

server.loadbanlist(server.banlist_file)
server.load_geoip_database(server.geoip_db_file)

server.log_status("-> Successfully loaded Hopmod")
