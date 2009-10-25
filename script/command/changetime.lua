--[[
	A player command to change the current map time
]]

return function(cn,minutes, seconds)

	if not minutes then
		return false, "#changetime <minutes> [<seconds>]"
	end
    
	minutes = tonumber(time)
    seconds = tonumber(time or 0)
    
	--if minutes >= 0 and time < 13670 then
    --    server.changetime(((time*60)*1000))
    --else
	--	return false, "max map.time is 13669"
	--end
    
    server.changetime((minutes*60*1000)+seconds*1000)
end
