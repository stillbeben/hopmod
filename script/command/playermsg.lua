-- #playermsg <cn> "<text>"

function server.playercmd_playermsg(playermsg_cn,playermsg_tcn,playermsg_text)
    return mastercmd(function()
	if not playermsg_tcn then
            server.player_msg(playermsg_cn,red("#playermsg <cn> <text>"))
        elseif not playermsg_text then
            server.player_msg(playermsg_cn,red("text is missing"))
        elseif not server.valid_cn(playermsg_tcn) then
            server.player_msg(playermsg_cn,red("cn is not valid"))
        else
	    server.player_msg(playermsg_tcn,"(" .. green("PM") .. ")  (" .. yellow(server.player_name(cn)) .. "): " .. playermsg_text)
        end
    end,playermsg_cn)
end
