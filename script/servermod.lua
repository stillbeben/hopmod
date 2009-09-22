load_once("restart")
load_once("player_object")
load_once("player_vars")
load_once("player_command")
load_once("logging")
load_once("maprotation")

server.module("db/auth")

function sendServerBanner(cn)

    if server.player_pvar(cn,"shown_banner") then return end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(1000,function()

        -- cancel if not the same player from 1 second ago
        if sid ~= server.player_sessionid(cn) then return end
        
        server.player_msg(cn, orange(server.servername))
        server.player_msg(cn, server.motd)
        
        server.player_pvar(cn, "shown_banner", true)
    end)
end

function onConnect(cn)

    if server.player_status_code(cn) == server.SPECTATOR then
        local sid = server.player_sessionid(cn)
        server.sleep(10000, function()
            if sid == server.player_sessionid(cn) then sendServerBanner(cn) end
        end)
    end
    
    local country = server.ip_to_country(server.player_ip(cn))
 	
    if server.show_country_message == 1 and #country > 0 then
        
        local str_connect = string.format("%s connected from %s.", green(server.player_name(cn)), green(country))
        local str_connect_admin = str_connect .. " (IP: " .. red(server.player_ip(cn)) .. ")"
        
        for index, cn in ipairs(server.players()) do
            if server.player_priv_code(cn) == 2 then
                server.player_msg(cn, str_connect_admin)
            else
                server.player_msg(cn, str_connect)
            end
        end
    end
    
end

function onDisconnect(cn)

    server.player_unsetpvar(cn, "shown_banner")
    
    if tonumber(server.playercount) == 0 then
        if server.firstgame_on_empty == 1 then
            server.changemap(server.first_map, server.first_gamemode, -1)
        else
            local lmode = server.first_gamemode
            local lmap = server.first_map
            if (server.random_mode_on_empty == 1) and (server.random_map_on_empty == 1) then
                lmode = server.random_mode()
                lmap = server.random_map(lmode,1)
                server.changemap(lmap,lmode,-1)
            elseif server.random_mode_on_empty == 1 then
                lmode = server.random_mode()
                server.changemap(lmap,lmode,-1)
            elseif server.random_map_on_empty == 1 then
                lmap = server.random_map(lmode,1)
                server.changemap(lmap,lmode,-1)
            end
        end
    end
end

function onText(cn,text)
    
    local block = 0
    
    if block == 0 and server.player_pvar(cn,"mute") then
        server.player_msg(cn, red() .. "Your chat messages are being blocked.")
        block = -1
    end
    
    return block
end

function onMapVote(cn,map,mode)

    if server.player_priv_code(cn) == 2 then
        return
    end
    
    if server.allow_mapvote <= 0 then
        server.player_msg(cn, red() .. "Map voting is disabled.")
        return -1
    elseif (server.allow_modevote <= 0) and (mode ~= server.gamemode) then
        server.player_msg(cn, red() .. "Server only accepts votes for " .. server.gamemode .. " mode.")
        return -1
    elseif not ((server.disallow_coopedit == 0) and (mode == "coop edit")) then
        local isfound = 0
        for a,b in ipairs(table_unique(server.parse_list(server["game_modes"]))) do
            if mode == b then
                isfound = 1
                break
            end
        end
        if isfound == 0 then
            server.player_msg(cn,red("Server doesn't accept votes for " .. mode))
            return -1
        end
    end
    
    if (server.mapvote_disallow_unknown_map == 1) and not (mode == "coop edit") then
        local globalFound = server.is_known_map(map)
        local localFound = server.is_known_map(map, mode)
        
        if not globalFound then
            server.player_msg(cn, red() .. "\"" .. map .. "\" is an unknown map.")
            return -1
        end
        
        if not localFound then
            server.player_msg(cn, red()  .. "\"" .. map .. "\" is not a map you can play in " .. mode .. ".")
            return -1
	end
    end
end

server.event_handler("connect",onConnect)
server.event_handler("disconnect", onDisconnect)
server.event_handler("active", sendServerBanner)
server.event_handler("rename", function(cn) server.player_pvar(cn, "shown_banner", true) end)
server.event_handler("text",onText)
server.event_handler("sayteam", onText)
server.event_handler("mapvote", onMapVote)
server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

server.event_handler("mapchange", function(map, mode)
    
    if mode == "coop edit" and server.using_master_auth() and server.disable_masterauth_in_coopedit == 1 then
        
        server.use_master_auth(false)
        
        local handler
        handler = server.event_handler("mapchange", function(map, mode)
            
            if mode ~= "coop edit" then
            
                server.use_master_auth(true)
                server.cancel_handler(handler)
                
                for index, cn in ipairs(server.players()) do
                    if server.player_priv_code(cn) == 1 then
                        server.unsetmaster(cn)
                    end
                end
            end
            
        end)
        
        server.msg("Master is now available to the first player who types /setmaster 1")
    end
end)

server.event_handler("started", function()
    
    if server.fileExists("./conf/auth") then
        server.execCubeScriptFile("./conf/auth")
    end
    
    if server.fileExists(server.banlist_file) then
        server.execCubeScriptFile(server.banlist_file)
    end
    
    if server.use_script_socket_server == 1 then
    
        if not server["script_socket_supported?"]() then
            server.log_status("Cannot run script_socket server as it's not supported (couldn't detect libmicrohttpd as being installed at build time).")
        end
        
        server.script_socket_server(server.script_socket_port, server.script_socket_password)
    end
    
    if server.use_irc_bot == 1 then
        os.execute("bin/server start_ircbot")
        server.event_handler("shutdown", function() server.stop_ircbot() end)
    end
    
    server.reload_maprotation()
    
    if server.playercount == 0 then
    
        local lmode = server.first_gamemode
        local lmap = server.first_map
        
        if (server.random_mode_on_empty == 1) and (server.random_map_on_empty == 1) then
            lmode = server.random_mode()
            lmap = server.random_map(lmode,1)
            server.changemap(lmap,lmode,-1)
        elseif server.random_mode_on_empty == 1 then
            lmode = server.random_mode()
            server.changemap(lmap,lmode,-1)
        elseif server.random_map_on_empty == 1 then
            lmap = server.random_map(lmode,1)
            server.changemap(lmap,lmode,-1)
        else
            server.changemap(server.first_map, server.first_gamemode, -1)
        end
    end
    
    server.load_geoip_database(server.geoip_db_file)
   
	load_once("player_command_bindings")
    
    server.log_status("-> Successfully loaded Hopmod")
    
end)
