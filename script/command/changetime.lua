-- #changetime <time>

function server.playercmd_changetime(changetime_cn,changetime_time)
    return admincmd(function()
	local time = tonumber(changetime_time)
	if time > 0 and time < 13670 then
	    server.changetime(((time*60)*1000))
	else
	    server.player_msg(changetime_cn,red("max map.time is 13669"))
	end
    end,changetime_cn)
end

function server.playercmd_ctime(ctime_cn,ctime_time)
    server.playercmd_changetime(ctime_cn,ctime_time)
end
