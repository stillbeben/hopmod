--[[--------------------------------------------------------------------------
--
--    A script to record a demo of every game
--
--]]--------------------------------------------------------------------------

local is_recording = false

local function start_recording(map, mode)
    
	if mode == "coop edit" or server.playercount == 0 and is_recording then
        return
    end
    
    mode = string.gsub(mode, " ", "_")
    
    server.recorddemo(string.format("log/demo/%s.%s.%s.dmo", os.date("!%y_%m_%d.%H_%M"), mode, map))
    server.msg("recording demo")
    
    is_recording = true
end

server.event_handler("mapchange", start_recording)

server.event_handler("connect", function(cn)

	if server.playercount == 2 then
		start_recording(server.map, server.gamemode)
	end
end)

server.event_handler("disconnect", function(cn, reason)

	if server.playercount == 0 and is_recording == true then
		server.stopdemo()
		is_recording = false
	end
end)

server.event_handler("finishedgame", function()
	is_recording = false
end)

return {unload = function() end}
