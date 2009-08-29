
dofile("./script/serverlib.lua")
dofile("./script/logging.lua")
dofile("./script/maprotation.lua")

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

function onConnect(cn)

    local country = server.ip_to_country(server.player_ip(cn))
    
    if #country > 0 then
        server.msg(string.format("%s connected from %s.",green(server.player_name(cn)), green(country)))
    end
end

function onDisconnect(cn)

    server.player_unsetpvar(cn,"shown_banner")
    
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
    
    if server.process_player_command(cn,text) then block = -1 end
    
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



function onTeamkill(actor, victim)
    
    local teamkill_limit = server.teamkill_limit
    if teamkill_limit == -1 then return end
    
    if not server.player_var(actor,"shown_teamkill_warning") then
        
        if server.teamkill_showlimit == 1 then
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
server.event_handler("disconnect", onDisconnect)
server.event_handler("active", sendServerBanner)
server.event_handler("rename", function(cn) server.player_pvar(cn, "shown_banner", true) end)
server.event_handler("text",onText)
server.event_handler("sayteam", onText)
server.event_handler("teamkill", onTeamkill)
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
    
    dofile("./script/db/auth.lua")
    
    if server.fileExists("./conf/auth") then
        server.execCubeScriptFile("./conf/auth")
    end
    
    dofile("./script/playercmd.lua")
    
    if server.fileExists("./conf/bans") then
        dofile("./" .. server.banlist_file)
    end
    
    dofile("./script/db/stats.lua")
    
    if server.use_name_reservation == 1 then
        dofile("./script/db/nameprotect.lua")
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
    
    if server.teamkill_showlimit == 1 then
        server.playercmd_teamkills = function(cn)
            local tks = server.player_teamkills(cn)
            local noun = "teamkill"
            if tks ~= 1 then noun = noun .. "s" end
            server.player_msg(cn,string.format("%i %s.",tks,noun))
        end
    end
    
    if server.enable_dynamic_maxclients == 1 then
        dofile("./script/resize_server.lua")
    end
    
    if server.use_kickspec == 1 then
        dofile("./script/kickspec.lua")
    end 

    if server.enable_ownage_messages == 1 then
        dofile("./script/ownage.lua")
    end
    
    if server.enable_suddendeath == 1 then
        dofile("./script/suddendeath.lua")
    end
    
    if server.change_default_maptime == 1 then
	dofile("./script/change_default_maptime.lua")
    end
    
    if server.use_spec_inactives == 1 then
	dofile("./script/spec.inactives.lua")
    end
    
    if server.enable_ping_limiter == 1 then
        dofile("./script/pinglimiter.lua")
    end
    
    if server.use_cd_modmap == 1 then
        dofile("./script/cd_modmap.lua")
    end
    
    if server.use_cd_accuracy == 1 then
        dofile("./script/cd_accuracy.lua")
    end
    
    if server.use_cd_chainsawhack == 1 then
        dofile("./script/cd_chainsawhack.lua")
    end
    
    if server.use_resize_mastermode == 1 then
        dofile("./script/resize_server_mastermode.lua")
    end
    
    server.reload_maprotation()
    
    if tonumber(server.playercount) == 0 then
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

    server.log_status("-> Successfully loaded Hopmod")
    
end)
