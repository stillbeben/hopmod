-- [[ Player command written by Thomas

function server.playercmd_players(cn_client) -- list players + player stats
    for i, cn in ipairs(server.players()) do
	str = "Name: " .. server.player_name(cn) .. " Frags: " .. server.player_frags(cn) .. " Deaths: " .. server.player_deaths(cn) .. " Acc: "
        server.player_msg(cn_client, str)
    end
end

-- ]]
