load_once("base/restart")
load_once("base/player/object_wrapper")
load_once("base/player/global_vars")
load_once("base/player/command")
load_once("base/logging")
load_once("base/maprotation") -- the reason this can be loaded now instead as a scheduled module is because of reload_maprotation is called from started event
load_once("base/mute")
load_once("base/banner")

server.module("db/auth")
server.module("base/mapvote")

server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

local function update_gamemodeinfo()
    gamemodeinfo = server.gengamemodeinfo()
    server.gamemodeinfo = gamemodeinfo
end

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
    
    update_gamemodeinfo()
    
end)

update_gamemodeinfo()

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
    
    if server.change_default_maptime == 1 then
        load_once("change_default_maptime")
    end
    
    server.reload_maprotation()
    
    server.load_geoip_database(server.geoip_db_file)
    
    load_once("command/_bindings")
    log_unknown_player_commands()
    
    server.log_status("-> Successfully loaded Hopmod")
    
end)
