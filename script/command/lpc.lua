-- [[ Player command written by Thomas

return function(cn)
    for i, player in ipairs(server.players()) do
        server.player_msg(cn, "Player: " .. green(server.player_name(player)) .. " Country: " ..  orange(server.ip_to_country(server.player_ip(player))))
    end
end
