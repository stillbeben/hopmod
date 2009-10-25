--[[
	A player command to change the current map time
]]

return function(cn,minutes, seconds)

	if not minutes then
		return false, "#changetime <minutes> [<seconds>]"
	end
    
	minutes = tonumber(minutes)
    seconds = tonumber(seconds or 0)
    
    server.changetime((minutes*60*1000)+(seconds*1000))
end
