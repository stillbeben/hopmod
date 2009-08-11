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
	    server.player_msg(warning_cn,red("cn is not valid"))
	else
	    local warns_count = (server.player_vars(warning_tcn).warnings_count or 1)
	    if warns_count <= server.warnings_limit then
		server.player_msg(warning_tcn," ")
		if warns_count == server.warnings_limit then
		    server.player_msg(warning_tcn,"(" .. red("Last warning") .. ")  " .. orange(warning_text))
		else
		    server.player_msg(warning_tcn,"(" .. red("Warning") .. ")  " .. orange(warning_text))
		end
		server.player_msg(warning_tcn," ")
		server.player_vars(warning_tcn).warnings_count = warns_count + 1
	    else
		server.kick(warning_tcn,3600,warning_cn,"")
		server.player_vars(warning_tcn).warnings_count = nil
	    end
        end
    end, warning_cn)
end 

function server.playercmd_warn(warn_cn,warn_tcn,warn_text)
    server.playercmd_warning(warn_cn,warn_tcn,warn_text)
end
