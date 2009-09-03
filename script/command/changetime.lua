-- #changetime <time>

local cmd_changetime = {}

function cmd_changetime.playercmd(changetime_cn,changetime_time)
    local time = tonumber(changetime_time)
    if time > 0 and time < 13670 then
        server.changetime(((time*60)*1000))
    else
        server.player_msg(changetime_cn,red("max map.time is 13669"))
    end
end

if server.changetime_command_master == 1 then
    function server.playercmd_changetime(changetime_cn,changetime_time)
	return mastercmd(function()
	    cmd_changetime.playercmd(changetime_cn,changetime_time)
	end,changetime_cn)
    end
else
    function server.playercmd_changetime(changetime_cn,changetime_time)
	return admincmd(function()
	    cmd_changetime.playercmd(changetime_cn,changetime_time)
	end,changetime_cn)
    end
end


function server.playercmd_ctime(ctime_cn,ctime_time)
    server.playercmd_changetime(ctime_cn,ctime_time)
end
