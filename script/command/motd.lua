-- #motd "<text>"

function server.playercmd_motd(cn, text)
    return admincmd(function()
        if not text then
            server.player_msg(cn,red("#motd \"<text>\""))
        end
        server.motd = text
        server.player_msg(cn,"MOTD changed to " .. text)
    end, cn)
end
