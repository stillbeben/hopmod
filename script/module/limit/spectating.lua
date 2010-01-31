--[[--------------------------------------------------------------------------
--
--    A script to kick spectators
--    who reach the spectate time limit and the server is full
--
--]]--------------------------------------------------------------------------


local diff = server.kickspec_players_diff
local max_spec_time = server.kickspec_max_time
local max_spec_time_when_server_empty = server.kickspec_max_spec_time_when_server_empty

local interval_check_time = server.kickspec_interval_check_time

local event = {}

local is_unload = false


-- maptime - timeplayed = return [in seconds]
local function spectime(maptime, cn)

	return (maptime - (server.player_timeplayed(cn) * 1000))

end


-- kick and clear mem-place
local function kick(cn)

	server.kick(cn, "0", "server", "spec-time too high")
	server.player_pvars(cn).spec_time = nil

end


-- spec time check
local function check_spec_times(is_empty)

	local max_stime = max_spec_time

	if is_empty then
		max_stime = max_spec_time_when_server_empty
	end

	for p in server.splayers() do
		if (p:pvars().spec_time or 0) > max_stime then
			kick(p.cn)
		end
	end

end


event.disconnect = server.event_handler_object("disconnect", function(cn, reason)

	server.player_pvars(cn).spec_time = nil

end)

-- notice new spectators, remove leaving persons
event.spectator = server.event_handler_object("spectator", function(cn, joined)

	if joined == 0 then
		server.player_pvars(cn).spec_time = nil
	end

end)


-- set the spectimes on "intermission"
event.finishedgame = server.event_handler_object("finishedgame", function()

	local gamelimit = server.gamelimit

	for p in server.splayers() do
		p:pvars().spec_time = (p:pvars().spec_time or 0) + spectime(gamelimit, p.cn)
	end

end)


-- check spec times every X minutes
server.interval(interval_check_time, function()

	if is_unload == true then
		return -1
	end

	local pdiff = diff
	if (server.maxclients - pdiff) >= server.maxclients then
		pdiff = server.maxclients - 1
	end

	if server.mastermode == 0 and server.playercount > (server.maxclients - pdiff) then
		check_spec_times()
	end

	if (server.playercount - server.speccount) == 0 then
		check_spec_times("is_empty")
	end

end)


-- unloader
local function unload()

	is_unload = true

	event = {}

	for p in server.aplayers() do
		p:pvars().spec_time = nil
	end

end


return {unload = unload}
