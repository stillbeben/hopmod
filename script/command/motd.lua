-- #motd "<text>"

return function(cn,text)

    if not text then
        server.player_msg(cn,red("#motd \"<text>\""))
    end
    
    server.motd = text
    server.player_msg(cn,"MOTD changed to " .. text)
    
end
