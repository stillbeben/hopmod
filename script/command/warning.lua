-- #warning [<cn>] "<text>"

function server.playercmd_warning(warning_cn,warning_tcn,warning_text)
    return mastercmd(function()
	if not warning_tcn then
	    server.player_msg(warning_cn,red("#warning [<cn>] \"<text>\""))
	    return
	elseif not warning_text then
    	    if warning_tcn >= "0" and warning_tcn <= "999" then
    		server.player_msg(warning_cn,red("text is missing"))
    	    else
    		server.msg(" ")
    		server.msg("(" .. red("Warning") .. ")  " .. orange(warning_tcn))
    		server.msg(" ")
    	    end
	elseif not server.valid_cn(warning_tcn) then
	    server.player_msg(warning_tcn," ")
            server.player_msg(warning_tcn,"(" .. red("Warning") .. ")  " .. orange(warning_text))
	    server.player_msg(warning_tcn," ")
	else
	    server.player_msg(warning_cn,red("cn is not valid"))
        end
    end, cn)
end 

function server.playercmd_warn(warn_cn,warn_tcn,warn_text)
    server.playercmd_warning(warn_cn,warn_tcn,warn_text)
end
