-- helper
--
-- send message and set person to spectator
local function modmap_stresser(modmap_stresser_cn)
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  You have a " .. red("modified map"))
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  " .. orange("You cannot play on this map!") .. " Please, wait for the next map")
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  and " .. orange("re-download the game") .. " from " .. yellow("www.sauerbraten.org"))
    server.spec(modmap_stresser_cn)
end

-- start functon above and send message about user with modmap to everyone
local function modmap_checker(modmap_checker_cn)
    modmap_stresser(modmap_checker_cn)
    server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(modmap_checker_cn)) .. " has a " .. red("modified map"))
end


-- events
--
server.event_handler("disconnect",function(modmap_disconnect_cn)
    server.player_vars(modmap_disconnect_cn).modmap = false
end)

server.event_handler("finishedgame",function()
    for a,cn in ipairs(server.spectators()) do
	server.player_vars(cn).modmap = false
    end
end)

-- safe player, when he has a modmap [and mode is not coop and map is known]
server.event_handler("mapcrcfail",function(modmap_mapcrcfail_cn)
    if ( tonumber(server.playercount) >= 3 ) and ( not (server.gamemode == "coop edit") ) and ( server.is_known_map(server.map) ) then
	server.player_vars(modmap_mapcrcfail_cn).modmap = true
	modmap_checker(modmap_mapcrcfail_cn)
    end
end)

-- check safed player, when he tries to leave spectator
server.event_handler("spectator",function(modmap_spectator_cn,modmap_spectator_joined)
    if ( modmap_spectator_joined == 0 ) and ( server.player_vars(modmap_spectator_cn).modmap ) then
	modmap_stresser(modmap_spectator_cn)
    end
end)


-- admin-cmd: #unfspec <cn>
if server.enable_unfspec_command == 1 then
    function server.playercmd_unfspec(unfspec_cn,unfspec_tcn)
	local function unfspec_error_msg(unfspec_error_msg_cn)
	    server.player_msg(unfspec_error_msg_cn,red("usage: #unfspec <cn>"))
	end
	
	return admincmd(function()
	    if not unfspec_tcn then
		unfspec_error_msg(unfspec_cn)
	    else
		local tmp = tonumber(unfspec_tcn)
		if server.valid_cn(tmp) then
		    server.player_vars(tmp).modmap = false
		    server.unspec(tmp)
		else
		    unfspec_error_msg(unfspec_cn)
		end
	    end
	end,unfspec_cn)
    end
end
