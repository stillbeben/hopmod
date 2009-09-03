local cmd_unmute = {}

function cmd_unmute.playercmd(unmute_cn,unmute_tcn)
    if not unmute_tcn then
        server.player_msg(unmute_cn,red("#unmute <cn>"))
    elseif not server.valid_cn(unmute_tcn) then
        server.player_msg(unmute_cn,red("cn is not valid"))
    else
        server.unmute(unmute_tcn)
    end
end

if server.unmute_command_master == 1 then
    function server.playercmd_unmute(unmute_cn,unmute_tcn)
	return mastercmd(function()
    	    cmd_unmute.playercmd(unmute_cn,unmute_tcn)
	end,unmute_cn)
    end
else
    function server.playercmd_unmute(unmute_cn,unmute_tcn)
	return admincmd(function()
    	    cmd_unmute.playercmd(unmute_cn,unmute_tcn)
	end,unmute_cn)
    end
end
