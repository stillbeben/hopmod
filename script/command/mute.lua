-- #mute <cn>

local cmd_mute = {}

function cmd_mute.playercmd(mute_cn,mute_tcn)
    if not mute_tcn then
        server.player_msg(mute_cn,red("#mute <cn>"))
    elseif not server.valid_cn(mute_tcn) then
        server.player_msg(mute_cn,red("cn is not valid"))
    else
        server.mute(mute_tcn)
    end
end

if server.mute_command_master == 1 then
    function server.playercmd_mute(mute_cn,mute_tcn)
	return mastercmd(function()
    	    cmd_mute.playercmd(mute_cn,mute_tcn)
	end,mute_cn)
    end
else
    function server.playercmd_mute(mute_cn,mute_tcn)
	return admincmd(function()
    	    cmd_mute.playercmd(mute_cn,mute_tcn)
	end,mute_cn)
    end
end
