dofile("./script/serverlib.lua")
dofile("./script/logging.lua")
dofile("./script/maprotation.lua")
dofile("./script/playercmd.lua")

function onConnect(cn)
    local country = server.ip_to_country(server.player_ip(cn))
    if #country > 0 then
        server.msg(string.format("%s connected from %s.",green(server.player_name(cn)), green(country)))
    end
end

function onActive(cn)
    server.sleep(1000,function()
        server.player_msg(cn, orange() .. server.servername)
        server.player_msg(cn, server.motd)
    end)
end

function onText(cn,text)
    
    local block = 0
    
    if server.process_player_command(cn,text) then block = -1 end
    
    if block == 0 and server.player_pvar(cn,"mute") then
        server.player_msg(cn, red() .. "Your chat messages are being blocked.")
        block = -1
    end
    
    return block
end

function onMapVote(cn,map,mode)
    if tonumber(server.allow_mapvote) <= 0 then
        server.player_msg(cn, red() .. "Map voting is disabled.")
        return -1
    else
        if mode ~= server.gamemode and tonumber(server.allow_modevote) <= 0 then
            server.player_msg(cn, red() .. "Server only accepts votes for " .. server.gamemode .. " mode.")
            return -1
        end
    end
end

server.event_handler("connect",onConnect)
server.event_handler("active", onActive)
server.event_handler("text",onText)
server.event_handler("sayteam", onText)
server.event_handler("mapvote", onMapVote)
server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

if tonumber(server.use_script_socket_server) == 1 then
    server.script_socket_server(server.script_socket_port)
end

server.loadbanlist(server.banlist_file)

server.load_geoip_database(server.geoip_db_file)

server.log_status("-> Successfully loaded Hopmod")
