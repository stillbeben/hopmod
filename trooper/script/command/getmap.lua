
return function(cn, mode)

    if not mode then
        mode = server.gamemode
    else
        mode = server.parse_mode(mode)
        
        if not mode then
            return false, "<mode> is not known"
        elseif mode == "coop edit" then
            return false, "coop edit is not supported."
        end
    end
    
    server.player_msg(cn, "random map: " .. green(server.random_map(mode)) .. " (" .. green(mode) .. ")")
    
end
