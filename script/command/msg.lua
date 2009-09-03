-- #msg "<text>"

local cmd_msg = {}

function cmd_msg.playercmd(msg_cn,msg_text)
    if not msg_text then
        server.player_msg(msg_cn,red("#msg \"<text>\""))
    else
        server.msg("(" .. green("Info") .. ")  " .. msg_text)
    end
end

if server.msg_command_master == 1 then
    function server.playercmd_msg(msg_cn,msg_text)
	return mastercmd(function()
    	    cmd_msg.playercmd(msg_cn,msg_text)
	end,msg_cn)
    end
else
    function server.playercmd_msg(msg_cn,msg_text)
	return admincmd(function()
    	    cmd_msg.playercmd(msg_cn,msg_text)
	end,msg_cn)
    end
end
