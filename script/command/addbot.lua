-- #addbot [<low>] [<high>]

local cmd_addbot = {}

local function usage_msg(cn)
    server.player_msg(cn, red("#addbot [<low>] [<high>]"))
end

return function(cn, arg1, arg2)

    local low = 70
    local high = 90
    local tmp = nil
    
    if arg2 then
        tmp = tonumber(arg2)
        if tmp > 0 and tmp <= 101 then
	    high = tmp
	else
	    usage_msg(cn)
	    return
	end
	tmp = tonumber(arg1)
	if tmp > 0 and tmp <= high then
	    low = tmp
	else
	     usage_msg(cn)
	    return
	end
    elseif arg1 then
        tmp = tonumber(arg1)
        if tmp > 0 and tmp <= 101 then
	    high = tmp
	    if low > high then
	        low = high
	    end
	else
	     usage_msg(cn)
	    return
	end
    end
    if low == high then
        server.addbot(low)
    else
        server.addbot(math.random(low,high))
    end
end
