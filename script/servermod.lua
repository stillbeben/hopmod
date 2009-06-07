dofile("./script/serverlib.lua")
dofile("./script/logging.lua")
dofile("./script/maprotation.lua")
dofile("./script/playercmd.lua")

function sendServerBanner(cn)

    if server.player_pvar(cn,"shown_banner") then return end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(1000,function()

        -- cancel if not the same player from 1 second ago
        if sid ~= server.player_sessionid(cn) then return end
        
        server.player_msg(cn, orange() .. server.servername)
        server.player_msg(cn, server.motd)
        
        server.player_pvar(cn, "shown_banner", true)
    end)
end

server.event_handler("rename", function(cn) server.player_pvar(cn, "shown_banner", true) end)
server.event_handler("active", sendServerBanner)
server.event_handler("disconnect", function(cn) server.player_unsetpvar(cn,"shown_banner") end)

function onConnect(cn)
    local country = server.ip_to_country(server.player_ip(cn))
    if #country > 0 then
        server.msg(string.format("%s connected from %s.",green(server.player_name(cn)), green(country)))
    end
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
    
    if server.player_priv_code(cn) == 2 then return end -- admin player
    
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

function onTeamkill(actor, victim)
    
    local teamkill_limit = tonumber(server.teamkill_limit)
    if teamkill_limit == -1 then return end
    
    if not server.player_var(actor,"shown_teamkill_warning") then
        
        if tonumber(server.teamkill_showlimit) == 1 then
            server.player_msg(actor,"This server will not tolerate more than " .. teamkill_limit .. " team kills per game.")
        else
            server.player_msg(actor,"This server enforces a team kill limit, and so you need to play more carefully. You have been warned.")
        end
        
        server.player_var(actor,"shown_teamkill_warning",true)
    end
    
    if server.player_teamkills(actor) > teamkill_limit then
        server.kick(actor,1800,"server","teamkilling")
    end
    
end

server.event_handler("connect",onConnect)
server.event_handler("text",onText)
server.event_handler("sayteam", onText)
server.event_handler("teamkill", onTeamkill)
server.event_handler("mapvote", onMapVote)
server.event_handler("shutdown",function() server.log_status("Server shutting down.") end)

server.event_handler("mapchange", function(map, mode)
    
    if mode == "coop edit" and server.using_master_auth() and tonumber(server.disable_masterauth_in_coopedit) == 1 then
        
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
    
    dofile("./script/db/auth.lua")
    server.auth_add_user = auth.add_user
    server.auth_add_domain = auth.add_domain
    
    dofile("./script/db/stats.lua")
    
    if tonumber(server.use_script_socket_server) == 1 then
        if not server["script_socket_supported?"]() then
            server.log_status("Cannot run script_socket server as it's not supported (couldn't detect libmicrohttpd as being installed at build time).")
        end
        server.script_socket_server(server.script_socket_port, server.script_socket_password)
    end
    
    if tonumber(server.use_irc_bot) == 1 then
        os.execute("bin/server start_ircbot")
        server.event_handler("shutdown", function() server.stop_ircbot() end)
    end
    
    if tonumber(server.teamkill_showlimit) == 1 then
        server.playercmd_teamkills = function(cn)
            local tks = server.player_teamkills(cn)
            local noun = "teamkill"
            if tks ~= 1 then noun = noun .. "s" end
            server.player_msg(cn,string.format("%i %s.",tks,noun))
        end
    end
    
    if tonumber(server.enable_dynamic_maxclients) == 1 then
        dofile("./script/resize_server.lua")
    end
    
    server.reload_maprotation()
    
    if tonumber(server.playercount) == 0 then
        server.changemap(server.first_map,server.first_gamemode,-1)
    end
    
    server.loadbanlist(server.banlist_file)

    server.load_geoip_database(server.geoip_db_file)
    
    server.log_status("-> Successfully loaded Hopmod")
    
end)

