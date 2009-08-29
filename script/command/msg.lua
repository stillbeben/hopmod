-- #msg "<text>"

function server.playercmd_msg(msg_cn,msg_text)
    return mastercmd(function()
        if not msg_text then
            server.player_msg(msg_cn,red("#msg \"<text>\""))
        else
    	    server.msg("(" .. green("Info") .. ")  " .. msg_text)
    	end
    end,msg_cn)
end
