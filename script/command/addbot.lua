-- #addbot [<low>] [<high>]

function server.playercmd_addbot(cn,arg1,arg2)
    local function error(cn)
	server.player_msg(cn,red("#addbot [<low>] [<high>]"))
    end
    
    return mastercmd(function()
	local low = 70
	local high = 90
	local tmp = nil
	if arg2 then
	    tmp = tonumber(arg2)
	    if tmp > 0 and tmp <= 101 then
		high = tmp
	    else
		error(cn)
		return
	    end
	    tmp = tonumber(arg1)
	    if tmp > 0 and tmp <= high then
		low = tmp
	    else
		error(cn)
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
		error(cn)
		return
	    end
	end
	if low == high then
	    server.addbot(low)
	else
	    server.addbot(math.random(low,high))
	end
    end,cn)
end
