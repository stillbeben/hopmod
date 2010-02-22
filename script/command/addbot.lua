--[[

	A player command to add bots

]]


local usage = "#addbot [<low>] [<high>]"


return function(cn,arg1,arg2)

	local low = 70
	local high = 90
	local tmp = nil

	if arg2 then

		tmp = tonumber(arg2)

		if tmp > 0 and tmp <= 101 then

			high = tmp

		else

			return false, usage
		end

		tmp = tonumber(arg1)

		if tmp > 0 and tmp <= high then

			low = tmp

		else

			return false, usage
		end

	elseif arg1 then

		tmp = tonumber(arg1)

		if tmp > 0 and tmp <= 101 then

			high = tmp

			if low > high then

				low = high
			end

		else

			return false, usage
		end
	end

	if low == high then

		server.addbot(low)

	else

		server.addbot(math.random(low,high))
	end

end
