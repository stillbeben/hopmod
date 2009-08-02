-- helper
--
-- give teamsize
local function teamsize(teamsize_teamname)
    local size = 0
    for a,cn in ipairs(server.team_players(teamsize_teamname)) do
	if not ( server.player_status_code(cn) == 5 ) then
	    size = size + 1
	end
    end
    return size
end

-- give teamsize, notice bots
local function teamsize_bot(teamsize_bot_teamname)
    local size = teamsize(teamsize_bot_teamname)
    local a = tonumber(server.botcount)
    while a > 0 do
	local tmp = 127 + a
	if server.player_team(tmp) == teamsize_bot_teamname then
	    size = size + 1
	end
	a = a - 1
    end
    return size
end

-- give other team's name
local function other_team(other_team_teamname)
    if (other_team_teamname == "evil") then
	return "good"
    else
	return "evil"
    end
end

-- give fuller team's name
local function fuller_team()
    if (teamsize("good") > teamsize("evil")) then
	return "good"
    else
	return "evil"
    end
end

-- player-difference between the teams
local function team_diff()
    return (math.abs(teamsize("good") - teamsize("evil")))
end

-- player-difference between the teams, notice bots
local function team_diff_bot()
    return (math.abs(teamsize_bot("good") - teamsize_bot("evil")))
end

-- check balance
local function teams_unbalanced()
    if team_diff() > 0 then
	return true
    else
	return false
    end
end

-- addbot
local function teambalance_addbot()
    server.addbot(-1)
    server.msg("(" .. green("Info") .. ")  " .. orange("random skilled bot has joined"))
end

-- delbot and del all bots
local function teambalance_delbot(option)
    if not option then
        local tmp = 127 + tonumber(server.botcount)
        if not ( tmp == teambalance_flag_cn1 or tmp == teambalance_flag_cn2 ) then
            server.delbot()
            server.log("server removed bot")
        else
            server.sleep(1500,function()
        	teambalance_delbot()
            end)
	end
    else
        while tonumber(server.botcount) > 0 do
            teambalance_delbot()
        end
    end
end

-- add/ delbot, when balance forces it
local function teambalance()
    local fuller = fuller_team()
    local other = other_team(fuller)
    if teams_unbalanced() then
	while teamsize_bot(other) < teamsize_bot(fuller) do
	    teambalance_addbot()
	end
	while (tonumber(server.botcount) > 0) and (teamsize_bot(other) > teamsize(fuller)) do
	    teambalance_delbot()
	end
    else
	teambalance_delbot("all")
    end
end

-- delay balance checks
local function teambalance_balance_checker(option)
    if not option then
	option = 500
    end
    server.sleep(option,function()
	teambalance()
    end)
end

-- event-functions
--
-- balance check and at leaving spec: re-team, when it is good for balance
local function teambalance_spectator_event(teambalance_spec_cn,teambalance_spec_joined)
    if teambalance_spec_joined == 1 then
	teambalance_balance_checker(5000)
    else
	if team_diff() > 1 then
	    spec_fteam = fuller_team()
	    if server.player_team(teambalance_spec_cn) == spec_fteam then
		server.changeteam(teambalance_spec_cn,other_team(spec_fteam))
		server.player_msg(teambalance_spec_cn,"(" .. green("Info") .. ")  " .. orange("you switched the team for balance"))
	    end
	end
	teambalance_balance_checker()
    end
end

-- reject other names than good and evil and disallow unbalancing
local function teambalance_chteamrequest_event(teambalance_chteam_cn,teambalance_chteam_old,teambalance_chteam_new)
    if not (teambalance_chteam_new == "good" or teambalance_chteam_new == "evil") then
	return (-1)
    elseif not ( server.player_status_code(teambalance_chteam_cn) == 5 ) then
	if (math.abs((teamsize(teambalance_chteam_old) - 1) - (teamsize(teambalance_chteam_new) + 1))) > 1 then
	    server.player_msg(teambalance_chteam_cn,"(" .. green("Info") .. ")  " .. orange("don't unbalance the teams"))
	    return (-1)
	end
	teambalance_balance_checker()
    end
end

local function teambalance_text_event(teambalance_text_cn,teambalance_text_text)
    if teams_unbalanced() then
	text_fteam = fuller_team()
	if server.player_team(teambalance_text_cn) == text_fteam then
	    if ( server.player_status(teambalance_text_cn) == "dead" ) and ( string.match(teambalance_text_text,"balance") or string.match(teambalance_text_text,"BALANCE") ) then
		server.changeteam(teambalance_text_cn,otherteam(text_fteam))
		server.player_msg(teambalance_text_cn,"(" .. green("Info") .. ")  " .. orange("you switched the team for balance"))
	    end
	end
	teambalance_balance_checker()
    end
end

local function teambalance_dropflag_event(teambalance_dropflag_cn)
    if teambalance_flag_cn1 == tonumber(teambalance_dropflag_cn) then
        teambalance_flag_cn1 = -1
    else
        teambalance_flag_cn2 = -1
    end
end

local function teambalance_takeflag_event(teambalance_takeflag_cn)
    if teambalance_flag_cn1 == -1 then
        teambalance_flag_cn1 = tonumber(teambalance_takeflag_cn)
    else
        teambalance_flag_cn2 = tonumber(teambalance_takeflag_cn)
    end
end

local function teambalance_disconnect_event(teambalance_disconnect_cn)
    teambalance_dropflag_event(teambalance_disconnect_cn)
    teambalance_balance_checker(10000)
end


-- enable/disable teambalancer
--
local function teambalance_check_mode(teambalance_check_mode_mode)
    if teambalance_check_mode_mode == "instagib" or teambalance_check_mode_mode == "tactics" or teambalance_check_mode_mode == "ffa" or teambalance_check_mode_mode == "efficiency" or teambalance_check_mode_mode == "coop edit" then
	teambalance_disabler()
    else
	teambalance_enabler()
    end
end

function teambalance_enabler()
    teambalance_flag_cn1 = -1
    teambalance_flag_cn2 = -1
    server.botbalance = 1
    teambalance_delbot("all")
    
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
    
    if teambalance_events_active == 0 then
	teambalance_events_active = 1
	
	local teambalance_connect = server.event_handler("connect",function(cn)
		teambalance_balance_checker()
	    end)
	local teambalance_disconnect = server.event_handler("disconnect",teambalance_disconnect_event)
	local teambalance_spectator = server.event_handler("spectator",teambalance_spectator_event)
	local teambalance_chteamrequest = server.event_handler("chteamrequest",teambalance_chteamrequest_event)
	local teambalance_text = server.event_handler("text",teambalance_text_event)
	local teambalance_intermission = server.event_handler("intermission",function()
	        teambalance_disabler()
	    end)
	local teambalance_takeflag = server.event_handler("takeflag",teambalance_takeflag_event)
	local teambalance_dropflag = server.event_handler("dropflag",teambalance_dropflag_event)
	
	teambalance_events = {}
	table.insert(teambalance_events,teambalance_connect)
	table.insert(teambalance_events,teambalance_disconnect)
	table.insert(teambalance_events,teambalance_spectator)
	table.insert(teambalance_events,teambalance_chteamrequest)
	table.insert(teambalance_events,teambalance_text)
	table.insert(teambalance_events,teambalance_intermission)
	
	local gmode = tostring(server.gamemode)
	if gmode == "insta ctf" or gmode == "insta protect" or gmode == "ctf" or gmode == "protect" then
	    table.insert(teambalance_events,teambalance_takeflag)
	    table.insert(teambalance_events,teambalance_dropflag)
	end
    end
    
    teambalance_balance_checker(10000)
end

function teambalance_disabler()
    if teambalance_events_active == 1 then
	teambalance_events_active = 0
	
	for i,handlerId in ipairs(teambalance_events) do
	    server.cancel_handler(handlerId)
	end
	teambalance_events = {}
    end
    
    teambalance_flag_cn1 = -1
    teambalance_flag_cn2 = -1
    teambalance_delbot("all")
    server.botbalance = teambalance_botbalance_isactive
end


-- always running events
--
server.event_handler("setmastermode",function(old,teambalance_setmastermode_new)
    if teambalance_setmastermode_new == "open" then
	teambalance_check_mode(tostring(server.gamemode))
    else
	teambalance_disabler()
    end
end)

server.event_handler("mapchange",function(map,teambalance_mapchange_mode)
    if tonumber(server.mastermode) == 0 then
	teambalance_check_mode(teambalance_mapchange_mode)
    else
	teambalance_disabler()
    end
end)


-- on start
--
teambalance_events_active = 0
teambalance_flag_cn1 = -1
teambalance_flag_cn2 = -1
teambalance_botbalance_isactive = tonumber(server.botbalance)
teambalance_enabler()
