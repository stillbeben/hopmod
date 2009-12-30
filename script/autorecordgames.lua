--[[--------------------------------------------------------------------------
--
--    A script to record a demo of every game
--
--]]--------------------------------------------------------------------------


local event = {}

server.recordgames_active = true

local is_recording = false


local function start_recording(map, mode)

	local gmode = mode

	if not (gmode == "coop edit" or server.playercount == 0) and server.recordgames_active == true and is_recording == false then

		if gmode == "tactics team" then
			gmode = "tactics_team"
		elseif gmode == "efficiency team" then
			gmode = "efficiency_team"
		elseif gmode == "instagib team" then
			gmode = "instagib_team"
		elseif gmode == "insta ctf" then
			gmode = "insta_ctf"
		elseif gmode == "insta protect" then
			gmode = "insta_protect"
		elseif gmode == "regen capture" then
			gmode = "regen_capture"
		end

		server.recorddemo("log/demo/" .. os.date("!%y_%m_%d.%H_%M") .. "." .. gmode .. "." .. server.map .. ".dmo")
		server.msg("Start demo recording")
		is_recording = true

	end

end


event.mapchange = server.event_handler_object("mapchange", start_recording)


event.connect = server.event_handler_object("connect", function(cn)

	if server.playercount == 2 then
		start_recording(server.map, server.gamemode)
	end

end)


event.disconnect = server.event_handler_object("disconnect", function(cn, reason)

	if server.playercount == 0 and is_recording == true then
		server.stopdemo()
		is_recording = false
	end

end)


event.finishedgame = server.event_handler_object("finishedgame", function()

	is_recording = false

end)


local function unload()

	event = {}
	server.recordgames_active = nil
	is_recording = false

end


return {unload = unload}
