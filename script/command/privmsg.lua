-- #privmsg <cn> "<text>"

function server.playercmd_privmsg(privmsg_cn,privmsg_tcn,privmsg_text)
    if not privmsg_tcn then
	server.player_msg(privmsg_cn,red("#privmsg <cn> \"<text>\""))
    elseif not privmsg_text then
        server.player_msg(privmsg_cn,red("text is missing"))
    elseif not server.valid_cn(privmsg_tcn) then
    	server.player_msg(privmsg_cn,red("cn is not valid"))
    else
	server.player_msg(privmsg_tcn,"(" .. green("PM") .. ")  (" .. green(server.player_name(privmsg_cn)) .. "(" .. magenta(privmsg_cn) .. ")): " .. privmsg_text)
    end
end 

function server.playercmd_pmsg(pmsg_cn,pmsg_tcn,pmsg_text)
    server.playercmd_privmsg(pmsg_cn,pmsg_tcn,pmsg_text)
end

function server.playercmd_pm(pm_cn,pm_tcn,pm_text)
    server.playercmd_privmsg(pm_cn,pm_tcn,pm_text)
end
