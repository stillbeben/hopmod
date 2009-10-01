--[[

	A player command to change the current map time

]]


return function(cn,time)

	if not time then

		return false, "#changetime <time>"
	end

	time = tonumber(time)

	if time >= 0 and time < 13670 then

		server.changetime(((time*60)*1000))

	else

		return false, "max map.time is 13669"
	end

end
