-- #uptime

-- [[ Player command written by Thomas

function server.playercmd_uptime(cn)
    server.player_msg(cn, "Server-Uptime: " .. server.format_duration(server.uptime / 1000))
end

-- ]]
