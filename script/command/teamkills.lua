
return function(cn)

    local teamkills = server.player_teamkills(cn)
    
    local noun = "teamkill"
    if teamkills ~= 1 then noun = noun .. "s" end
    
    server.player_msg(cn, string.format("%i %s", teamkills, noun))
    
end
