-- #privmsg <cn> "<text>"

return function(privmsg_cn, privmsg_tcn, privmsg_text)
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
