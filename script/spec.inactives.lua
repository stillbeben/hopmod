local spec_inactives_check_active = 0
local spec_inactives_events_active = 0
local spec_inactives_events = {}
local spec_inactives_time = round((server.spec_inactives_time / 1000),0)
local unloadEventHandlers
local still_loaded = true

if server.spec_inactives_check_time <= 0 then
    server.spec_inactives_check_time = 1000
end

local function spec_inactives_set_deathtime(spec_inactives_set_deathtime_cn)
    server.player_vars(spec_inactives_set_deathtime_cn).spec_inactives_deathtime = server.player_connection_time(spec_inactives_set_deathtime_cn)
end

local function loadEventHandlers()

	local event_frag = server.event_handler("frag",function(spec_inactives_frag_event_tcn,spec_inactives_frag_event_acn)
		spec_inactives_set_deathtime(spec_inactives_frag_event_tcn)
    end)
    
	local event_suicide = server.event_handler("suicide",function(spec_inactives_suicide_event_cn)
		spec_inactives_set_deathtime(spec_inactives_suicide_event_cn)
    end)
    
    unloadEventHandlers = function()
    
        server.cancel_handler(event_frag)
        server.cancel_handler(event_suicide)
        
        still_loaded = false
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
    
    if not still_loaded then return -1 end
    
	for player in server.gplayers() do
        
	    local deathtime_limit = server.player_vars(cn).spec_inactives_deathtime
        
	    if deathtime_limit then
            if player:status_code(cn) == 1 then
                if player:connection_time(cn) - deathtime_limit >= spec_inactives_time then
                
                    player:spec()
                    player:msg(cn,"(" .. green("Info") .. ")  you are inactive, therefore you joined spectators.")
                    player:vars(cn).spec_inactives_deathtime = nil
                end
            else
                player:vars(cn).spec_inactives_deathtime = nil
            end
        end
	end

end)

loadEventHandlers()

return {unload = unloadEventHandlers}
