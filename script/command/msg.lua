-- #msg "<text>"

return function(msg_cn,msg_text)
    if not msg_text then
        server.player_msg(msg_cn, red("#msg \"<text>\""))
    else
        server.msg("(" .. green("Info") .. ")  " .. msg_text)
    end
end
