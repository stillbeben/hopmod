local sudden_death = 0
local sudden_death_events = {}
local sudden_death_events_active = 0
local sudden_death_ctl_events = {}
local sudden_death_ctl_events_active = 0

sudden_death_events_active = 0
sudden_death_ctl_events_active = 0

local function sd_frag_scoreflag_event_helper()
    if sudden_death == 1 then
        sudden_death = 0
        server.changetime(10)
    end
end

local function sd_frag_event(tcn,acn)
    sd_frag_scoreflag_event_helper()
server.msg("blab")
end

local function sd_scoreflag_event(cn)
    sd_frag_scoreflag_event_helper()
end

local function sd_no_event()
    server.interval(2000,function()
	if sudden_death == 1 then
	    local break_first = 0
	    local draw = 0
	    for index,name in ipairs(server.teams()) do
		local tscore = server.team_score(name)
		for i,n in ipairs(server.teams()) do
                    if not (name == n) then
			if tscore == server.team_score(n) then
			    draw = 1
			    break_first = 1
			    break
			end
		    end
		end
		if break_first == 1 then
		    break
		end
	    end
	    if draw == 0 then
		sudden_death = 0
		server.changetime(10)
		return (-1)
	    end
	end
	if sudden_death == 0 then
    	    return (-1)
    	end
    end)
end

local function sd_disabler()
    if sudden_death_events_active == 1 then
	sudden_death_events_active = 0
	
	for a,b in ipairs(sudden_death_events) do
	    server.cancel_handler(b)
	end
	sudden_death_events = {}
    end
    sudden_death = 0
end

local function sd_finishedgame_event()
    sd_disabler()
end

local function sd_enabler(sd_enabler_option)
server.msg("blib")
    if not sd_enabler_option then
	return (-1)
    end
    if sudden_death_events_active == 0 then
server.msg("blib")
	sudden_death_events_active = 1
	
	local event_frag = nil
	local event_scoreflag = nil
	sudden_death_events = {}
	if (sd_enabler_option == 0) or (sd_enabler_option == 3) then
server.msg("blib")
	    event_frag = server.event_handler("frag",sd_frag_event)
	    table.insert(sudden_death_events,event_frag)
	elseif sd_enabler_option == 1 then
	    sd_no_event()
	elseif sd_enabler_option == 2 then
	    event_scoreflag = server.event_handler("scoreflag",sd_scoreflag_event)
	    table.insert(sudden_death_events,event_scoreflag)
	end
server.msg("blib")
	local event_finishedgame = server.event_handler("finishedgame",sd_finishedgame_event)
	table.insert(sudden_death_events,event_finishedgame)
    end
    sudden_death = 1
end


local function sd_timeupdate_event(sd_timeupdate_event_mins)
    if sd_timeupdate_event_mins == 1 then
        server.sleep(58000,function()
	    local gmode = tostring(server.gamemode)
server.msg(gmode)
	    local break_first = 0
	    local draw = 1
	    if gmode == "regen capture" or gmode == "capture" then
server.msg("blobs")
		for index,name in ipairs(server.teams()) do
		    local tscore = server.team_score(name)
		    for i,n in ipairs(server.teams()) do
                	if not (name == n) then
			    if not (tscore == server.team_score(n)) then
				draw = 0
				break_first = 1
				break
			    end
			end
		    end
		    if break_first == 1 then
			break
		    end
		end
		if draw == 1 then
		    server.msg(red("--[ Sudden Death. Next Score Wins!"))
		    sd_enabler(1)
		end
	    elseif gmode == "insta ctf" or gmode == "ctf" or gmode == "insta protect" or gmode == "protect" then
		for index,name in ipairs(server.teams()) do
		    local tscore = server.team_score(name)
		    for i,n in ipairs(server.teams()) do
			if not (name == n) then
			    if not (tscore == server.team_score(n)) then
				draw = 0
				break_first = 1
				break
			    end
			end
		    end
		    if break_first == 1 then
			break
		    end
		end
		if draw == 1 then
		    server.msg(red("--[ Sudden Death. Next Score Wins!"))
		    sd_enabler(2)
		end
	    elseif gmode == "teamplay" or gmode == "tactics team" or gmode == "instagib team" or gmode == "efficiency team" then
server.msg("blob")
		for index,name in ipairs(server.teams()) do
		    local tscore = server.team_score(name)
		    for i,n in ipairs(server.teams()) do
			if not (name == n) then
			    if not (tscore == server.team_score(n)) then
				draw = 0
				break_first = 1
				break
			    end
			end
		    end
		    if break_first == 1 then
			break
		    end
		end
		if draw == 1 then
		    server.msg(red("--[ Sudden Death. Next Frag Wins!"))
		    sd_enabler(3)
		end
	    elseif gmode == "ffa" or gmode == "tactics" or gmode == "instagib" or gmode == "efficiency" then
		for index,cn in ipairs(server.players()) do
		    local pfrags = server.player_frags(cn)
		    for i,c in ipairs(server.players()) do
			if not (cn == c) then
			    if not (pfrags == server.player_frags(c)) then
				draw = 0
				break_first = 1
				break
			    end
			end
		    end
		    if break_first == 1 then
			break
		    end
		end
		if draw == 1 then
		    server.msg(red("--[ Sudden Death. Next Frag Wins!"))
server.msg(blue("mod"))
		    sd_enabler(0)
		end
	    end
        end)
    end
    if sudden_death == 1 then
	return 1
    else
	return sd_timeupdate_event_mins
    end
end

local function sd_ctl_disabler()
    if sudden_death_ctl_events_active == 1 then
	sudden_death_ctl_events_active = 0
	
	for a,b in ipairs(sudden_death_ctl_events) do
	    server.cancel_handler(b)
	end
	sudden_death_ctl_events = {}
    end
    sd_disabler()
end

local function sd_ctl_enabler()
    if sudden_death_ctl_events_active == 0 then
	sudden_death_ctl_events_active = 1
	
	sudden_death_ctl_events = {}
	local event_timeupdate = server.event_handler("timeupdate",sd_timeupdate_event)
	table.insert(sudden_death_ctl_events,event_timeupdate)
    end
end

function server.sdmode(sdmode_cn,sdmode_option)
    if sdmode_option == 1 then
        sd_ctl_enabler()
    else
        sd_ctl_disabler()
    end
end

-- #nosd
if server.enable_nosd_command == 1 then
    if server.enable_sd_command == 0 then
        server.log("WARNING: #nosd available, but not #sd")
    end
    function server.playercmd_nosd(nosd_cn)
	return admincmd(function()
            server.sdmode(nosd_cn,0)
            server.msg(orange("--[ Sudden Death Mode Disabled. There may be ties"))
        end,nosd_cn)
    end
end

-- #sd
if server.enable_sd_command == 1 then
    if server.enable_nosd_command == 0 then
        server.log("WARNING: #sd available, but not #nosd")
    end
    function server.playercmd_sd(sd_cn)
	return admincmd(function()
            server.sdmode(sd_cn,1)
            server.msg(orange("--[ Sudden Death Mode Enabled. There will be no ties"))
        end,sd_cn)
    end
end
