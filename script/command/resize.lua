-- #resize [<number>]

local cmd_resize = {}

function cmd_resize.playercmd(resize_cn,resize_size)
    if not resize_size then
        server.maxplayers = cmd_resize.normalmaxplayers
    elseif tonumber(resize_size) < 0 then
        server.player_msg(resize_cn,red("size is not an allowed number"))
    else
        if tonumber(resize_size) < server.resize_totalminplayers then
            resize_size = server.resize_totalminplayers
        elseif tonumber(resize_size) > server.resize_totalmaxplayers then
            resize_size = server.resize_totalmaxplayers
        end
        server.maxplayers = resize_size
    end
end

server.event_handler("disconnect",function(cn)
    if tonumber(server.playercount) == 0 then
        server.maxplayers = cmd_resize.normalmaxplayers
    end
end)

cmd_resize.normalmaxplayers = server.maxplayers

if server.resize_command_master == 1 then
    function server.playercmd_resize(resize_cn,resize_size)
	return mastercmd(function()
    	    cmd_resize.playercmd(resize_cn,resize_size)
	end,resize_cn)
    end
else
    function server.playercmd_resize(resize_cn,resize_size)
	return admincmd(function()
    	    cmd_resize.playercmd(resize_cn,resize_size)
	end,resize_cn)
    end
end
