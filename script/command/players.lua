-- [[ Player command written by Thomas

return function(source) -- list players + player stats
    for i, cn in ipairs(server.players()) do 
        str = "Name: " .. server.player_name(cn) .. " Frags: " .. server.player_frags(cn) .. " Deaths: " .. server.player_deaths(cn) .. " Acc: " .. server.player_accuracy (cn)
        server.player_msg(source, str)
    end
end
