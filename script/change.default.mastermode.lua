local function set_default_mastermode()
    ldfm = tonumber(server.default_mastermode)
    server.mastermode = ldfm
    
    -- disable/ activate kickspec, when it was active
    if set_default_mastermode_kickspec_isactive == 1 then
	if ldfm == 0 then
	    kickspec_enabler()
	else
	    kickspec_disabler()
	end
    end
    -- disable/ activate teambalancer, when it was active
    if set_default_mastermode_teambalancer_isactive == 1 then
	if ldfm == 0 then
	    teambalance_check_mode(tostring(server.gamemode))
	else
	    teambalance_disabler()
	end
    end
end

-- on start
-- check if kickspec is active
local set_default_mastermode_kickspec_isactive = server.use_kickspec
-- check if teambalancer is active
local set_default_mastermode_teambalancer_isactive = server.use_teambalancer

set_default_mastermode()

-- events
server.event_handler("setmaster",function(cn)
    set_default_mastermode()
end)

server.event_handler("disconnect",function(cn)
    if tonumber(server.playercount) == 0 then
	set_default_mastermode()
    end
end)
