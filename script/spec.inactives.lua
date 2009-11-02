--[[--------------------------------------------------------------------------
--
--    A script to let players join spectors
--        who are dead and reach the time limit
--    when mastermode is open
--            
--]]--------------------------------------------------------------------------

if server.spec_inactives_check_time <= 0 then
	server.spec_inactives_check_time = 1000
end

spec_inactives = {}	-- namespace

spec_inactives.time = round((server.spec_inactives_time / 1000),0)

spec_inactives.check_active = false

spec_inactives.eventFrag = nil
spec_inactives.eventSuicide = nil

spec_inactives.still_loaded = true
spec_inactives.unloadEventHandlers = nil


function spec_inactives.setDeathtime(cn)

	server.player_vars(cn).spec_inactives_deathtime = server.player_connection_time(cn)

end


function spec_inactives.enable()

	if not spec_inactives.eventFrag then
		spec_inactives.eventFrag = server.event_handler("frag",function(tcn,acn)

			spec_inactives.setDeathtime(tcn)

		end)
	end

	if not spec_inactives.eventSuicide then
		spec_inactives.eventSuicide = server.event_handler("suicide",function(cn)

			spec_inactives.setDeathtime(cn)

		end)
	end

	spec_inactives.check_active = true

end


function spec_inactives.disable()

	spec_inactives.check_active = false

	if spec_inactives.eventFrag then
		server.cancel_handler(spec_inactives.eventFrag)
		spec_inactives.eventFrag = nil
	end

	if spec_inactives.eventSuicide then
		server.cancel_handler(spec_inactives.eventSuicide)
		spec_inactives.eventSuicide = nil
	end

end


function spec_inactives.loadEventHandlers()

	spec_inactives.still_loaded = true

	spec_inactives.enable()

	if not spec_inactives.unloadEventHandlers then
		spec_inactives.unloadEventHandlers = function()

			spec_inactives.disable()

			spec_inactives.still_loaded = false

		end
	end
    
end


server.event_handler("setmastermode",function(cn,old,new)

	if new == "open" then
 		spec_inactives.enable()
	else
		spec_inactives.disable()
	end

end)


server.event_handler("mapchange",function(map,mode)

	if tonumber(server.mastermode) == 0 then
		spec_inactives.enable()
	else
		spec_inactives.disable()
	end

end)


server.interval(server.spec_inactives_check_time,function()
    
	if not spec_inactives.still_loaded then
		return -1
	end

	if spec_inactives.check_active == true then
		for p in server.gplayers() do
			local deathtime = p:vars().spec_inactives_deathtime

			if deathtime then
				if p:status_code() == 1 then
					if p:connection_time() - deathtime >= spec_inactives.time then
						p:spec()
						p:msg("(" .. green("Info") .. ")  you joined spectators, because you are inactive - type " .. yellow("/setmaster 0") .. " to rejoin the game.")
						p:vars().spec_inactives_deathtime = nil
					end
				else
					p:vars().spec_inactives_deathtime = nil
				end
			end
		end
	end

end)

spec_inactives.loadEventHandlers()

return {unload = spec_inactives.unloadEventHandlers}
