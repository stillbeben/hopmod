
local function failed_action(cn)
    server.player_vars(cn).modmap = server.map
    -- force_spec is no good idea - no way to decide when a player might be allowed to unspec again
    server.spec(cn)
end

server.event_handler("modmap", function(cn, map, crc)

    if map ~= server.map or server.gamemode == "coop edit" then
        return
    end

    server.log(string.format("%s(%i) is using a modified map: %s (crc %s)", server.player_name(cn), cn, map, crc))  
    failed_action(cn)

end)
-- block mapmod player, when he tries to leave spectator
server.event_handler("spectator",function(cn,joined)

    if ( joined == 0 ) and ( server.player_vars(cn).modmap == server.map ) then
        failed_action(cn)
    end

end)
