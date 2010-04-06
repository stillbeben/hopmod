
function msg_admins(cn, msg)
    
    local line = string.format("%s: %s", server.player_displayname(cn), green(msg))
    
    for _, cn in ipairs(server.clients()) do
        if server.player_priv_code(cn) == server.PRIV_ADMIN then
            server.player_msg(cn, line)
        end
    end
end

server.event_handler("text", function(cn, text)
    if server.player_status_code(cn) == server.SPECTATOR and server.player_priv_code(cn) ~= server.PRIV_ADMIN then
        msg_admins(cn, text)
        return -1
    end
end)
