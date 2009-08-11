local spec_inactives_check_active = 0
local spec_inactives_events_active = 0
local spec_inactives_events = {}
local spec_inactives_time = round((server.spec_inactives_time / 1000),0)

if server.spec_inactives_check_time <= 0 then
    server.spec_inactives_check_time = 1000
end


local function spec_inactives_set_deathtime(spec_inactives_set_deathtime_cn)
    server.player_vars(spec_inactives_set_deathtime_cn).spec_inactives_deathtime = server.player_connection_time(spec_inactives_set_deathtime_cn)
end


local function spec_inactives_enabler()
    if spec_inactives_events_active == 0 then
	spec_inactives_check_active = 1
	spec_inactives_events_active = 1
	
	local event_frag = server.event_handler("frag",function(spec_inactives_frag_event_tcn,spec_inactives_frag_event_acn)
		spec_inactives_set_deathtime(spec_inactives_frag_event_tcn)
	    end)
	local event_suicide = server.event_handler("suicide",function(spec_inactives_suicide_event_cn)
		spec_inactives_set_deathtime(spec_inactives_suicide_event_cn)
	    end)
	
	spec_inactives_events = {}
	table.insert(spec_inactives_events,event_frag)
	table.insert(spec_inactives_events,event_suicide)
    end
end

local function spec_inactives_disabler()
    if spec_inactives_events_active == 1 then
	spec_inactives_events_active = 0
	spec_inactives_check_active = 0
	
	for i,handlerId in ipairs(spec_inactives_events) do
	    server.cancel_handler(handlerId)
	end
	spec_inactives_events = {}
    end
end


server.event_handler("setmastermode",function(spec_inactives_setmastermode_event_old,spec_inactives_setmastermode_event_new)
    if spec_inactives_setmastermode_event_new == "open" then
        spec_inactives_enabler()
    else
        spec_inactives_disabler()
    end
end)

server.event_handler("mapchange",function(spec_inactives_mapchange_event_map,spec_inactives_mapchange_event_mode)
    if tonumber(server.mastermode) == 0 then
        spec_inactives_enabler()
    else
        spec_inactives_disabler()
    end
end)


server.interval(server.spec_inactives_check_time,function()
    if spec_inactives_check_active == 1 then
	for a,cn in ipairs(server.players()) do
	    local deathtime = server.player_vars(cn).spec_inactives_deathtime
	    if deathtime then
		if (server.player_status_code(cn) == 1) then
		    if (server.player_connection_time(cn) - deathtime) >= spec_inactives_time then
			server.spec(cn)
			server.player_msg(cn,"(" .. green("Info") .. ")  you are inactive, therefore you joined spectators.")
			server.player_vars(cn).spec_inactives_deathtime = nil
		    end
		else
		    server.player_vars(cn).spec_inactives_deathtime = nil
		end
	    end
	end
    end
end)


spec_inactives_check_active = 0
spec_inactives_events_active = 0
spec_inactives_enabler()
