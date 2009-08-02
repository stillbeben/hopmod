-- #duel <mode> (<cn1> <cn2> [<team1> <team2>] | <team1> <team2>) [<map> [forcemap]] [<time>] [nosd]

local running = false

local player1_cn = nil
local player1_id = nil
local player1_ready = false

local player2_cn = nil
local player2_id = nil
local player2_ready = false

local gamecount = 0
local suspended = false

local evthandlers = {}



local function onActive(cn)
    local id = server.player_id(cn)

    if id == player1_id then
        player1_ready = true
        if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	    if duel_teams_set == 0 or duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		server.changeteam(player1_cn,"evil")
	    else
		server.changeteam(player1_cn,duel_p1_team)
	    end
	end
    elseif id == player2_id then
        player2_ready = true
        if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	    if duel_teams_set == 0 or duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		server.changeteam(player2_cn,"good")
	    else
		server.changeteam(player2_cn,duel_p2_team)
	    end
	end
    end

    if player1_ready and player2_ready then
        if suspended then
            if id == match_master then server.setmaster(cn) end
        	server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Connected ]--"))
        	local countdown = 5
    		server.interval(1000, function()
            	    countdown = countdown - 1
    		    server.msg(orange(string.format("--[ Resuming the game in %i seconds ]--", countdown)))
		    if countdown == 0 then
    			suspended = false
    			server.pausegame(false)
        		return -1
    		    end
		end)
    	    else
    		if duel_lc == 1 then
        	    server.pausegame(false)
        	    server.changetime(duel_maptime)
        	    server.msg(orange("--[ Game started! ]--"))
        	    server.msg(green("--[ good luck and have fun ]--"))
        	    duel_lc = 0
        	end
    	    end
	end
    end

local function onMapchange(map, mode)
    gamecount = gamecount + 1
    if gamecount > 1 then
        running = false
        uninstallHandlers()
        return
    end

    server.pausegame(true)

    local lmode = duel_mode
    if lmode == "instagib team" then
	lmode = "instagib_team"
    elseif lmode == "tactics team" then
	lmode = "tactics_team"
    elseif lmode == "efficiency team" then
	lmode = "efficiency_team"
    elseif lmode == "insta ctf" then
	lmode = "insta_ctf"
    elseif lmode == "insta protect" then
	lmode = "insta_protect"
    elseif lmode == "regen capture" then
	lmode = "regen_capture"
    end
    server.recorddemo("log/demo/" .. player1_name .. ".vs." .. player2_name .. "." .. lmode .. "." .. map .. ".dmo")
    server.msg(orange("--[ Starting Demo Recording ]--"))

    server.msg(orange("--[ Waiting until all Players loaded the Map. ]--"))
end

local function onIntermission()
    if duel_la == 1 then
	server.msg(orange("--[ Good Game! ]--"))
	duel_la = 0
    end
    if server.gamemodeinfo.teams then
	if duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
    	    duel_p1_score = server.team_score("evil")
	    duel_p2_score = server.team_score("good")
	else
	    duel_p1_score = server.team_score(duel_p1_team)
	    duel_p2_score = server.team_score(duel_p2_team)
	end
    else
	duel_p1_score = server.player_frags(player1_cn)
	duel_p2_score = server.player_frags(player2_cn)
    end
    if duel_p1_score > duel_p2_score then
	duel_winner = player1_cn
	duel_winner_name = player1_name
	duel_winner_score = duel_p1_score
	duel_loser = player2_cn
	duel_loser_name = player2_name
	duel_loser_score = duel_p2_score
	duel_winner_team = duel_p1_team
	duel_loser_team = duel_p2_team
    elseif duel_p1_score < duel_p2_score then
	duel_winner = player2_cn
	duel_winner_score = duel_p2_score
	duel_winner_name = player2_name
	duel_loser = player1_cn
	duel_loser_score = duel_p1_score
	duel_loser_name = player1_name
	duel_winner_team = duel_p2_team
	duel_loser_team = duel_p1_team
    else
    	duel_winner = player1_cn
	duel_winner_score = duel_p1_score
	duel_winner_name = player1_name
	duel_loser = player2_cn
	duel_loser_score = duel_p2_score
	duel_loser_name = player2_name
	duel_winner_team = duel_p1_team
	duel_loser_team = duel_p2_team
    end
    if duel_lb == 1 then
	if duel_p1_score == duel_p2_score then
	    server.msg(green("--[ 1on1 game ended - " .. red("draw") .. " game! ]--"))
	    if duel_teams_set == 1 then
		server.log(string.format("MATCH: draw game between %s (%s) and %s (%s)",duel_winner_name,duel_winner_team,duel_loser_name,duel_loser_team))
		server.msg(string.format("Result: %s (%s) -|- %s (%s) -|- %s",red(duel_winner_name),red(duel_winner_team),blue(duel_loser_name),blue(duel_loser_team),green(duel_winner_score)))
	    else
		server.log(string.format("MATCH: draw game between %s and %s",duel_winner_name,duel_loser_name))
		server.msg(string.format("Result: %s vs %s -|- %s",red(duel_winner_name),blue(duel_loser_name),green(duel_winner_score)))
	    end
    	elseif duel_teams_set == 1 then
	    server.log(string.format("MATCH: %s (%s) wins with %s - %s (%s) has %s",duel_winner_name,duel_winner_team,duel_winner_score,duel_loser_name,duel_loser_team,duel_loser_score))
	    server.msg(green("--[ duel game ended - " .. red(duel_winner_name) .. " (" .. red(duel_winner_team) .. ") won the game! ]--"))
	    server.msg(string.format("Result: %s (%s) - %s  -|-  %s (%s) - %s",red(duel_winner_name),red(duel_winner_team),green(duel_winner_score),blue(duel_loser_name),blue(duel_loser_team),red(duel_loser_score)))
	else
	    server.log(string.format("MATCH: %s wins with %s - %s has %s",duel_winner_name,duel_winner_score,duel_loser_name,duel_loser_score))
	    server.msg(green("--[ 1on1 game ended - (" .. red(duel_winner_name) .. ") won the game! ]--"))
	    server.msg(string.format("Result: %s - %s  -|-  %s - %s",red(duel_winner_name),green(duel_winner_score),blue(duel_loser_name),red(duel_loser_score)))
	end
	duel_lb = 0
    end
    running = false
    sudden_death = false
    sudden_death_enabled = false
--    server.mastermode = 1
    server.reassignteams = reassignteams_isactive
    if server.gamemodeinfo.teams and not ( duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" ) then
	server.sleep(15000,function()
	    local flag = 0
	    for a,cn in ipairs(server.players()) do
	        local pteam = server.player_team(cn)
	        if not ( (pteam == "good") or (pteam == "evil") ) then
	            if flag == 0 then
	        	server.changeteam(cn,"good")
	                flag = 1
	            else
	                server.changeteam(cn,"evil")
	        	flag = 0
	            end
	        end
	    end
	end)
    end
end

local function onConnect(cn)
    local id = server.player_id(cn)

    if id == player1_id then
        player1_cn = cn
        player1_ready = false
        server.unspec(cn)
        if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	    if duel_teams_set == 0 or duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		server.changeteam(player1_cn,"evil")
	    else
		server.changeteam(player1_cn,duel_p1_team)
	    end
	end
    end

    if id == player2_id then
        player2_cn = cn
        player2_ready = false
        server.unspec(cn)
        if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	    if duel_teams_set == 0 or duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		server.changeteam(player2_cn,"good")
	    else
		server.changeteam(player2_cn,duel_p2_team)
	    end
	end
    end
end

local function onOut(cn,option)
    if not option then
	return
    else
	local id = server.player_id(cn)
	if id == player1_id or id == player2_id then
    	    if option == "disconnect" then
    		if player1_id == id then
    		    server.setmaster(player2_cn)
    		else
    		    server.setmaster(player1_cn)
    		end -- Handover master
    		server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Disconnected - Pausing Game ]--"))
    		player_left = 1
    		suspended = true
    	    elseif option == "spectator" then
    		server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Joined spectators - Pausing Game ]--"))
    	    end
    	    server.pausegame(true)
    	end
    end
end

local function onSpectator(cn,joined)
    local lcn = tonumber(cn)
    if joined == 1 then
	if lcn == player1_cn or lcn == player2_cn then
	    onOut(cn,"spectator")
	else
	    server.player_msg(lcn,string.format("(%s)  %s",green("Info"),orange("a duel is running, therefore your are muted")))
	    server.mute(lcn)
	end
    elseif joined == 0 then
	if lcn == player1_cn or lcn == player2_cn then
	    server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Left spectators ]--"))
    	    local countdown = 5
    	    server.interval(1000, function()
        	countdown = countdown - 1
    		server.msg(orange(string.format("--[ Resuming the game in %i seconds ]--", countdown)))
		if countdown == 0 then
    		    server.pausegame(false)
        	    return -1
    		end
	    end)
	else
	    server.unmute(cn)
	end
    end
end

local function onDisconnect(cn)
    onOut(cn,"disconnect")
end

local function onTimeUpdate(mins)
    if sudden_death_enabled == true then
        if fragged == true then
    	    fragged = false
    	    return 0
    	end
        if mins == 0 then
            for index, cn in ipairs(server.players()) do
                player_score[index] = server.player_frags(cn)
            end
            if player_score[1] == player_score[2] then
                server.msg(red("--[ Sudden Death. Next Frag Wins! ]--"))
                sudden_death = true
                return 1
            end
        end
        if sudden_death == true then
    	    return 1
    	else
    	    return mins
    	end
    else
        return mins
    end
end

local function onFrag()
    if sudden_death_enabled  == true then
        if sudden_death == true then
            sudden_death = false
            fragged = true
            server.changetime(0)
        end
    end
end

local function onChteamrequest(cn,old,new)
    local lcn = tonumber(cn)
    local lcn_status = server.player_status_code(lcn)
    if not ( lcn_status == 5 ) then
	if lcn == player1_cn then
	    if duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		if not ( new == "evil" ) then
		    return (-1)
		end
    	    elseif not ( new == duel_p1_team ) then
		return (-1)
	    end
	elseif lcn == player2_cn then
	    if duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		if not ( new == "good" ) then
		    return (-1)
		end
    	    elseif not ( new == duel_p2_team ) then
		return (-1)
	    end
	else
	    return (-1)
	end
    end
end


local function installHandlers()
    if duel_teams_set == 1 then
	server.servername = orig_servername .. " | duel: " .. duel_p1_team .. " - " .. duel_p2_team
    else
	server.servername = orig_servername .. " | 1on1: " .. player1_name .. " - " .. player2_name
    end

    duel_la = 1
    duel_lb = 1
    duel_lc = 1

    local connect = server.event_handler("connect",onConnect)
    local disconnect = server.event_handler("disconnect",onDisconnect)
    local active = server.event_handler("active",onActive)
    local mapchange = server.event_handler("mapchange",onMapchange)
    local intermission = server.event_handler("intermission",onIntermission)
    local frag = server.event_handler("frag",onFrag)
    local timeupdate = server.event_handler("timeupdate",onTimeUpdate)
    if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	chteamrequest = server.event_handler("chteamrequest",onChteamrequest)
    end

    table.insert(evthandlers,connect)
    table.insert(evthandlers,disconnect)
    table.insert(evthandlers,active)
    table.insert(evthandlers,mapchange)
    table.insert(evthandlers,intermission)
    table.insert(evthandlers,frag)
    table.insert(evthandlers,timeupdate)
    if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	table.insert(evthandlers,chteamrequest)
    end
end

function uninstallHandlers()
    server.servername = orig_servername

    for i,handlerId in ipairs(evthandlers) do
	server.cancel_handler(handlerId)
    end
    evthandlers = {}
    
    for a,cn in ipairs(server.players()) do
	server.unmute(cn)
    end
end


function server.playercmd_duel(cn,mode,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8)
     if running then
	if server.player_priv_code(cn) < 1 then
    	    if server.player_priv_code(player1_cn) > 0 or server.player_priv_code(player2_cn) > 0 then
        	server.player_msg(cn,red("Permission denied"))
        	return
    	    end
	end
    end
    
    if running then
        server.msg(red("--[ Master Cancelled the running versus match ]--"))
        player_score = {}
        sudden_death_enabled = false
        sudden_death = false
        running = false
        gamecount = 0
--        server.mastermode = 1
	server.reassignteams = reassignteams_isactive
        server.pausegame(false)
        uninstallHandlers()
        return
    end

    return mastercmd(function()
	orig_servername = tostring(server.servername)

	local function duel_error_msg(cn)
	    server.player_msg(cn,red("usage: #duel <mode> (<player1> <player2> [<team1> <team2>]|<team1> <team2>) [(<map>|random) [forcemap] [<map.time>] [nosd]"))
	end


	if not arg8 then
	    if not arg7 then
		if not arg6 then
		    if not arg5 then
			if not arg4 then
			    if not arg3 then
				if not arg2 then
				    duel_error_msg(cn)
				    return
				elseif server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = 300000
				    duel_map = "random"
				else
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = 300000
				    duel_map = "random"
				end
			    elseif arg3 == "nosd" then
				if server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = 300000
				    duel_map = "random"
				else
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = 300000
				    duel_map = "random"
				end
			    elseif ( arg3 > "0" ) and ( arg3 < "3600000" ) then
				if server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = tonumber(arg3) * 60000
				    duel_map = "random"
				else
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = tonumber(arg3) * 60000
				    duel_map = "random"
				end
			    else
				if server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = 300000
				    duel_map = arg3
				else
				    duel_nosd = 0
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = 300000
				    duel_map = arg3
				end
			    end
			elseif arg4 == "nosd" then
			    if ( arg3 > "0" ) and ( arg3 < "3600000" ) then
				if server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = tonumber(arg3) * 60000
				    duel_map = "random"
				else
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = tonumber(arg3) * 60000
				    duel_map = "random"
				end
			    else
				if server.valid_cn(arg2) and server.valid_cn(arg1) then
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 0
				    duel_p2_team = nil
				    duel_p1_team = nil
				    duel_players_set = 1
				    player2_cn = tonumber(arg2)
				    player1_cn = tonumber(arg1)
				    duel_maptime = 300000
				    duel_map = arg3
				else
				    duel_nosd = 1
				    duel_forcemap = 0
				    duel_teams_set = 1
				    duel_p2_team = arg2
				    duel_p1_team = arg1
				    duel_players_set = 0
				    player2_cn = -1
				    player1_cn = -1
				    duel_maptime = 300000
				    duel_map = arg3
				end
			    end
			elseif arg4 == "forcemap" then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 0
				duel_forcemap = 1
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = 300000
				duel_map = arg3
			    else
				duel_nosd = 0
				duel_forcemap = 1
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = 300000
				duel_map = arg3
			    end
			elseif ( arg4 > "0" ) and ( arg4 < "3600000" ) then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 0
				duel_forcemap = 0
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = tonumber(arg4) * 60000
				duel_map = arg3
			    else
				duel_nosd = 0
				duel_forcemap = 1
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = tonumber(arg4) * 60000
				duel_map = arg3
			    end
			else
			    duel_nosd = 0
			    duel_forcemap = 0
			    duel_teams_set = 1
			    duel_p2_team = arg4
			    duel_p1_team = arg3
			    duel_players_set = 1
			    player2_cn = tonumber(arg2)
			    player1_cn = tonumber(arg1)
			    duel_maptime = 300000
			    duel_map = "random"
			end
		    elseif arg5 == "nosd" then
			if arg4 == "forcemap" then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 1
				duel_forcemap = 1
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = 300000
				duel_map = arg3
			    else
				duel_nosd = 1
				duel_forcemap = 1
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = 300000
				duel_map = arg3
			    end
			elseif ( arg4 > "0" ) and ( arg4 < "3600000" ) then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 1
				duel_forcemap = 0
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = tonumber(arg4) * 60000
				duel_map = arg3
			    else
				duel_nosd = 1
				duel_forcemap = 0
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = tonumber(arg4) * 60000
				duel_map = arg3
			    end
			else
			    duel_nosd = 1
			    duel_forcemap = 0
			    duel_teams_set = 1
			    duel_p2_team = arg4
			    duel_p1_team = arg3
			    duel_players_set = 1
			    player2_cn = tonumber(arg2)
			    player1_cn = tonumber(arg2)
			    duel_maptime = 300000
			    duel_map = "random"
			end
		    elseif ( arg5 > "0" ) and ( arg5 < "3600000" ) then
			if arg4 == "forcemap" then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 0
				duel_forcemap = 1
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = tonumber(arg5) * 60000
				duel_map = arg3
			    else
				duel_nosd = 0
				duel_forcemap = 1
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = tonumber(arg5) * 60000
				duel_map = arg3
			    end
			else
			    duel_nosd = 0
			    duel_forcemap = 0
			    duel_teams_set = 1
			    duel_p2_team = arg4
			    duel_p1_team = arg3
			    duel_players_set = 1
			    player2_cn = tonumber(arg2)
			    player1_cn = tonumber(arg1)
			    duel_maptime = tonumber(arg5) * 60000
			    duel_map = "random"
			end
		    else
			duel_nosd = 0
			duel_forcemap = 0
			duel_teams_set = 1
			duel_p2_team = arg4
			duel_p1_team = arg3
			duel_players_set = 1
			player2_cn = tonumber(arg2)
			player1_cn = tonumber(arg1)
			duel_maptime = 300000
			duel_map = arg5
		    end
		elseif arg6 == "nosd" then
		    if ( arg5 > "0" ) and ( arg5 < "3600000" ) then
			if arg4 == "forcemap" then
			    if server.valid_cn(arg2) and server.valid_cn(arg1) then
				duel_nosd = 1
				duel_forcemap = 1
				duel_teams_set = 0
				duel_p2_team = nil
				duel_p1_team = nil
				duel_players_set = 1
				player2_cn = tonumber(arg2)
				player1_cn = tonumber(arg1)
				duel_maptime = tonumber(arg5) * 60000
				duel_map = arg3
			    else
				duel_nosd = 1
				duel_forcemap = 1
				duel_teams_set = 1
				duel_p2_team = arg2
				duel_p1_team = arg1
				duel_players_set = 0
				player2_cn = -1
				player1_cn = -1
				duel_maptime = tonumber(arg5) * 60000
				duel_map = arg3
			    end
			else
			    duel_nosd = 1
			    duel_forcemap = 1
			    duel_teams_set = 1
			    duel_p2_team = arg4
			    duel_p1_team = arg3
			    duel_players_set = 1
			    player2_cn = arg2
			    player1_cn = arg1
			    duel_maptime = tonumber(arg5) * 60000
			    duel_map = "random"
			end
		    else
			duel_nosd = 1
			duel_forcemap = 0
			duel_teams_set = 1
			duel_p2_team = arg4
			duel_p1_team = arg3
			duel_players_set = 1
			player2_cn = tonumber(arg2)
			player1_cn = tonumber(arg1)
			duel_maptime = 300000
			duel_map = arg5
		    end
		elseif arg6 == "forcemap" then
		    duel_nosd = 0
		    duel_forcemap = 1
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = 300000
		    duel_map = arg5
		elseif ( arg6 > "0" ) and ( arg6 < "3600000" ) then
		    duel_nosd = 0
		    duel_forcemap = 0
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = tonumber(arg6) * 60000
		    duel_map = arg5
		else
		    duel_error_msg(cn)
		    return
		end
	    elseif arg7 == "nosd" then
		if arg6 == "forcemap" then
		    duel_nosd = 1
		    duel_forcemap = 1
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = 300000
		    duel_map = arg5
		elseif ( arg6 > "0" ) and ( arg6 < "3600000" ) then
		    duel_nosd = 1
		    duel_forcemap = 0
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = tonumber(arg6) * 60000
		    duel_map = arg5
		else
		    duel_error_msg(cn)
		    return
		end
	    elseif ( arg7 > "0" ) and ( arg7 < "3600000" ) then
		if arg6 == "forcemap" then
		    duel_nosd = 0
		    duel_forcemap = 1
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = tonumber(arg7) * 60000
		    duel_map = arg5
		else
		    duel_error_msg(cn)
		    return
		end
	    else
		duel_error_msg(cn)
		return
	    end
	elseif arg8 == "nosd" then
	    if ( arg7 > "0" ) and ( arg7 < "3600000" ) then
		if arg6 == "forcemap" then
		    duel_nosd = 1
		    duel_forcemap = 1
		    duel_teams_set = 1
		    duel_p2_team = arg4
		    duel_p1_team = arg3
		    duel_players_set = 1
		    player2_cn = tonumber(arg2)
		    player1_cn = tonumber(arg1)
		    duel_maptime = tonumber(arg7) * 60000
		    duel_map = arg5
		else
		    duel_error_msg(cn)
		    return
		end
	    else
		duel_error_msg(cn)
		return
	    end
	else
	    duel_error_msg(cn)
	    return
	end
	duel_mode = mode

	if duel_mode == "insta" or duel_mode == "instagib" then
	    duel_mode = "instagib"
	elseif duel_mode == "tac" or duel_mode == "tactics" then
	    duel_mode = "tactics"
	elseif duel_mode == "ffa" then
	    duel_mode = "ffa"
	elseif duel_mode == "effic" or duel_mode == "efficiency" then
	    duel_mode = "efficiency"
	elseif duel_mode == "teamplay" then
	    duel_mode = "teamplay"
	elseif duel_mode == "efficteam" or duel_mode == "efficiency team" or duel_mode == "eteam" then
	    duel_mode = "efficiency team"
	elseif duel_mode == "tacteam" or duel_mode == "tactics team" or duel_mode == "tteam" then
	    duel_mode = "tactics team"
	elseif duel_mode == "instateam" or duel_mode == "instagib team" or duel_mode == "iteam" then
	    duel_mode = "instagib team"
	elseif duel_mode == "instactf" or duel_mode == "insta ctf" or duel_mode == "ictf" then
	    duel_mode = "insta ctf"
	elseif duel_mode == "ctf" then
	    duel_mode = "ctf"
	elseif duel_mode == "capture" then
	    duel_mode = "capture"
	elseif duel_mode == "regen" or duel_mode == "regencapture" or duel_mode == "regen capture" then
	    duel_mode = "regen capture"
	elseif duel_mode == "protect" then
	    duel_mode = "protect"
	elseif duel_mode == "instaprotect" or duel_mode == "insta protect" or duel_mode == "iprotect" then
	    duel_mode = "insta protect"
	else
	    server.player_msg(cn,red("gamemode is not known"))
	    return
	end

	if duel_map == "random" then
	    duel_map = server.random_map(duel_mode,1)
	elseif duel_forcemap == 0 then
	    if not ( server.is_known_map(duel_map) ) then
		server.player_msg(cn,orange("map is not known - set forcemap, if you want this map"))
		return
	    end
	end

	if not ( duel_maptime > 0 and duel_maptime < 3600000 ) then
	    server.player_msg(cn,red("invalid maptime"))
	    return
	end

	if duel_players_set == 1 then
	    if not ( server.valid_cn(player1_cn) and server.valid_cn(player2_cn) ) then
    		server.player_msg(cn,red("invalid cn given"))
		return
	    elseif player1_cn == player2_cn then
		server.player_msg(cn, red("player 1 and player 2 have the same cn"))
		return
	    end
	else
	    for a,cn in ipairs(server.players()) do
		if not (server.player_status_code(cn) == 5) then
	    	    if string.match(server.player_name(cn),duel_p1_team) then
	                player1_cn = cn
	                break
	            end
	        end
	    end
	    for a,cn in ipairs(server.players()) do
		if not (server.player_status_code(cn) == 5) then
	    	    if string.match(server.player_name(cn),duel_p2_team) then
	                player2_cn = cn
	                break
	            end
	        end
	    end
	    if (player1_cn == -1) or (player2_cn == -1) then
		server.player_msg(cn,red("player 1 or player 2 is not here or spectator"))
		return
	    end
	end

	if duel_teams_set == 1 then
	    if duel_mode == "instagib" then
		duel_mode = "instagib team"
	    elseif duel_mode == "tactics" then
		duel_mode = "tactics team"
	    elseif duel_mode == "ffa" then
		duel_mode = "teamplay"
	    elseif duel_mode == "efficiency" then
		duel_mode = "efficiency team"
	    end
	else
	    if duel_mode == "instagib team" then
		duel_mode = "instagib"
	    elseif duel_mode == "tactics team" then
		duel_mode = "tactics"
	    elseif duel_mode == "teamplay" then
		duel_mode = "ffa"
	    elseif duel_mode == "efficiency team" then
		duel_mode = "efficiency"
	    end
	end

	player_score = {}
	running = true
	gamecount = 0
	if duel_nosd == 0 then
	    if duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" or duel_mode == "instagib team" or duel_mode == "tactics team" or duel_mode == "teamplay" or duel_mode == "efficiency team" then
		sudden_death_enabled = true
	    else
		sudden_death_enabled = false
	    end
	else
	    sudden_death_enabled = false
	end
	sudden_death = false
	match_master = server.player_id(cn)

	player1_id = server.player_id(player1_cn)
	player2_id = server.player_id(player2_cn)
	player1_name = server.player_name(player1_cn)
	player2_name = server.player_name(player2_cn)

	server.specall()
	server.mastermode_owner = -1
	server.mastermode = 2
	-- disable teambalance, kickspec, server.reassignteams
	if server.use_teambalancer == 1 then
	    teambalance_disabler()
	end
	if server.use_kickspec == 1 then
	    kickspec_disabler()
	end
	reassignteams_isactive = tonumber(server.reassignteams)
	server.reassignteams = 0
	server.pausegame(true)

	server.msg(" ")
	if duel_teams_set == 0 or duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" then
	    server.log("--[ 1on1 - " .. player1_name .. " against " .. player2_name .. " mode: " .. duel_mode .. " map: " .. duel_map .. " map-time: " .. duel_maptime .. " mins ]--")
	    server.msg(green("--[ 1on1 - " .. red(player1_name) .. " against " .. blue(player2_name) .. " ]--"))
	else
	    server.log("--[ duel - " .. player1_name .. " (" .. duel_p1_team .. ") against " .. player2_name .. " (" .. duel_p2_team .. ") mode: " .. duel_mode .. " map: " .. duel_map .. " map-time: " .. duel_maptime .. " ]--")
	    server.msg(green("--[ duel - " .. red(player1_name) .. " (" .. red(duel_p1_team) .. ") against " .. blue(player2_name) .. " (" .. blue(duel_p2_team) .. ") ]--"))
	end
	server.msg(green("--[ mode: " .. orange(duel_mode) .. " map: " .. orange(duel_map) .. " map-time: " .. orange(duel_maptime / 60000) .. " mins ]--"))
	server.msg(" ")

	server.unspec(player1_cn)
	server.unspec(player2_cn)
	if not ( duel_mode == "instagib" or duel_mode == "tactics" or duel_mode == "ffa" or duel_mode == "efficiency" ) then
	    if duel_teams_set == 0 or duel_mode == "insta ctf" or duel_mode == "ctf" or duel_mode == "insta protect" or duel_mode == "protect" then
		server.changeteam(player1_cn,"evil")
		server.changeteam(player2_cn,"good")
	    else
		server.changeteam(player1_cn,duel_p1_team)
		server.changeteam(player2_cn,duel_p2_team)
	    end
	end

	installHandlers()

	server.sleep(5000,function()
	    local countdown = 5
	    server.interval(1000, function()
    		countdown = countdown - 1
    		server.msg(orange(string.format("-- [ Loading the map in %i seconds ]--", countdown)))
    		if countdown == 0 then
    		    server.sleep(1000,function()
        		server.changemap(duel_map,duel_mode,duel_maptime)
--			for a,cn in ipairs(server.spectators()) do
--			    server.player_msg(cn,string.format("(%s)  %s",green("Info"),orange("a duel is running, therefore your are muted")))
--			    server.mute(cn)
--			end
--			spectator = server.event_handler("spectator",onSpectator)
--			table.insert(evthandlers,spectator)
        	    end)
    		    return -1
    		end
	    end)
	end)
    end,cn)
end
