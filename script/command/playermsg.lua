-- #playermsg <cn> "<text>"

local cmd_playermsg = {}

function cmd_playermsg.playercmd(playermsg_cn,playermsg_tcn,playermsg_text)
    if not playermsg_tcn then
        server.player_msg(playermsg_cn,red("#playermsg <cn> <text>"))
    elseif not playermsg_text then
        server.player_msg(playermsg_cn,red("text is missing"))
    elseif not server.valid_cn(playermsg_tcn) then
        server.player_msg(playermsg_cn,red("cn is not valid"))
    else
        server.player_msg(playermsg_tcn,"(" .. green("PM") .. ")  (" .. yellow(server.player_name(cn)) .. "): " .. playermsg_text)
    end
end

if server.playermsg_command_master == 1 then
    function server.playercmd_playermsg(playermsg_cn,playermsg_tcn,playermsg_text)
	return mastercmd(function()
	    cmd_playermsg.playercmd(playermsg_cn,playermsg_tcn,playermsg_text)
	end,playermsg_cn)
    end
else
    function server.playercmd_playermsg(playermsg_cn,playermsg_tcn,playermsg_text)
	return admincmd(function()
	    cmd_playermsg.playercmd(playermsg_cn,playermsg_tcn,playermsg_text)
	end,playermsg_cn)
    end
end
