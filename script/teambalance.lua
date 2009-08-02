-- give teamsize
local function teamsize(teamsize_teamname)
    teamsize_i = 0
    for a,cn in ipairs(server.team_players(teamsize_teamname)) do
	teamsize_i = teamsize_i + 1
    end
    return teamsize_i
end

-- give other teamname
local function otherteam(otherteam_teamname)
    if (otherteam_teamname == "evil") then
	return "good"
    else
	return "evil"
    end
end

-- give fuller team's name
local function betterteam()
    if (teamsize("good") > teamsize ("evil")) then
	return "good"
    else
	return "evil"
    end
end

-- check balance
local function teambalance_teams_unbalanced()
    if (math.abs(teamsize("good") - teamsize("evil")) > 1) then
	return true
    else
	return false
    end
end

-- check team of dead player
local function teambalance_check_dead_player(check_dead_player_cn,check_dead_player_team)
    if server.player_team(check_dead_player_cn) == check_dead_player_team then
	return true
    else
	return false
    end
end

-- cancel events of search dead player
local function teambalance_cancel_search_dead_player_events()
    server.cancel_handler(search_dead_player_frag_event)
    server.cancel_handler(search_dead_player_suicide_event)
    search_dead_player_events_active = 0
end

-- check and move dead player
local function teambalance_move_dead_player(move_dead_player_cn)
    if teambalance_teams_unbalanced() then
	move_dead_player_bteam = (betterteam())
	if teambalance_check_dead_player(move_dead_player_cn,move_dead_player_bteam) then
	    server.changeteam(move_dead_player_cn,otherteam(move_dead_player_bteam))
    	    server.player_msg(move_dead_player_cn,string.format("(%s)  %s",green("Info"),orange("you switched the team for balance")))
    	    teambalance_cancel_search_dead_player_events()
    	    if teambalance_teams_unbalanced() then
    		teambalance_search_dead_player()
    	    end
    	end
    else
    	teambalance_cancel_search_dead_player_events()
    end
end

-- check already dead players
local function teambalance_check_already_dead_players()
    if teambalance_teams_unbalanced() then
	check_already_dead_players_bteam = (betterteam())
	check_already_dead_players_cn = 9999
	for a,cn in ipairs(server.team_players(check_already_dead_players_bteam)) do
            check_already_dead_players_cn = cn
            if server.player_status(check_already_dead_players_cn) == "dead" then
        	server.changeteam(check_already_dead_players_cn,otherteam(check_already_dead_players_bteam))
    		server.player_msg(check_already_dead_players_cn,string.format("(%s)  %s",green("Info"),orange("you switched the team for balance")))
    		search_dead_player_events_active = 0
                return false
            else
                check_already_dead_players_cn = 9999
            end
	end
	return true
    else
    	return true
    end
end

-- search dead player with frag/ suicide events
local function teambalance_search_dead_player()
    if teambalance_check_already_dead_players() then
	search_dead_player_frag_event = server.event_handler("frag",function(search_dead_player_frag_target,search_dead_player_frag_actor)
	    teambalance_move_dead_player(search_dead_player_frag_target)
	end)
	search_dead_player_suicide_event = server.event_handler("suicide",function(search_dead_player_suicide_cn)
	    teambalance_move_dead_player(search_dead_player_suicide_cn)
    	end)
    end
end

search_dead_player_events_active = 0
-- start balancechecker
local function teambalance_balance_checker()
    if tonumber(server.use_teambalancer) == 1 then
	if tonumber(search_dead_player_events_active) == 0 then
	    server.sleep(15000,function()
		search_dead_player_events_active = 1
		teambalance_search_dead_player()
	    end)
	end
    end
end

server.event_handler("disconnect",function(cn)
    teambalance_balance_checker()
end)

server.event_handler("spectator",function(teambalancer_spec_cn,teambalancer_spec_joined)
    if teambalancer_spec_joined == 1 then
	teambalance_balance_checker()
    elseif tonumber(server.use_teambalancer) == 1 then
	if teambalance_teams_unbalanced() then
	    spec_bteam = betterteam()
    	    if server.player_team(teambalancer_spec_cn) == spec_bteam then
    		server.changeteam(teambalancer_spec_cn,otherteam(betterteam()))
		server.player_msg(teambalancer_spec_cn,string.format("(%s)  %s",green("Info"),orange("you switched the team for balance")))
	    else
		teambalance_balance_checker()
	    end
	end
    end
end)

server.event_handler("chteamrequest",function(teambalancer_chteam_cn,teambalancer_chteam_old,teambalancer_chteam_new)
    if tonumber(server.use_teambalancer) == 1 then
	if not (teambalancer_chteam_new == "good" or teambalancer_chteam_new == "evil") then
	    return (-1)
	elseif not ( server.player_status_code(teambalancer_reteam_cn) == 5 ) then
	    if not ( teambalance_teams_unbalanced() ) then
	    	server.player_msg(teambalance_chteam_lcn,string.format("(%s)  %s",green("Info"),orange("don't unbalance the teams")))
	    	return (-1)
	    end
	end
    end
end)

server.event_handler("setmastermode",function(teambalancer_setmastermode_old,teambalancer_setmastermode_new)
    if not ( teambalancer_setmastermode_new == "open" ) then
	server.use_teambalancer = 0
    else
	server.use_teambalancer = 1
    end
end)

server.event_handler("text",function(teambalancer_text_cn,teambalancer_text_text)
    if tonumber(server.use_teambalancer) == 1 then
	if teambalance_teams_unbalanced() then
	    text_bteam = betterteam()
    	    if server.player_team(teambalancer_text_cn) == text_bteam then
    		if string.match(teambalancer_text_text,"balance") then
    		    if server.player_status(teambalancer_text_cn) == "dead" then
    			server.changeteam(teambalancer_text_cn,otherteam(text_bteam))
			server.player_msg(teambalancer_text_cn,string.format("(%s)  %s",green("Info"),orange("you switched the team for balance")))
		    end
		end
	    end
	end
    end
end)

local function teambalancer_check_mode()
    if tostring(server.gamemode) == "instagib" or tostring(server.gamemode) == "tactics" or tostring(server.gamemode) == "ffa" or tostring(server.gamemode) == "efficiency" or tostring(server.gamemode) == "coop edit" then
	server.use_teambalancer = 0
    else
	server.use_teambalancer = 1
    end
end

teambalancer_check_mode()

server.event_handler("mapchange",function(map,mode)
    teambalancer_check_mode()
end)
