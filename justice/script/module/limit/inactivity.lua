--[[
    A script to move inactive players to spectators
]]

local interval_time = server.spec_inactives_check_time

if interval_time <= 0 then
	interval_time = 60 * 1000
end

local inactive_time = round((server.spec_inactives_time / 1000), 0)
local is_unload = false

local function set_deathtime(cn)
	server.player_vars(cn).spec_inactives_deathtime = server.player_connection_time(cn)
end

server.event_handler("frag", set_deathtime) -- first argument of frag event is target cn
server.event_handler("suicide", set_deathtime)

server.interval(interval_time, function()

	if is_unload == true then
		return -1
	end

	if server.mastermode == 0 then
		for p in server.gplayers() do
			local deathtime = p:vars().spec_inactives_deathtime

			if deathtime then
				if p:status_code() == server.DEAD then
					if p:connection_time() - deathtime >= inactive_time then
						p:spec()
						p:msg("You joined spectators, because you seem to be inactive - type " .. yellow("/spectator 0") .. " to rejoin the game.")
						p:vars().spec_inactives_deathtime = nil
					end
				else
					p:vars().spec_inactives_deathtime = nil
				end
			end
		end
	end
	
end)

local function unload()

	is_unload = true

	for p in server.aplayers() do
		p:vars().spec_inactives_deathtime = nil
	end
end

return {unload = unload}
