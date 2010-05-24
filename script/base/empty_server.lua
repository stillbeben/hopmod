local start_gamemode = server.default_gamemode
local back_to_start = (server.default_game_on_empty == 1)

local function change_map()
    server.sleep(0, function()
        server.changemap(map_rotation.get_map_name(start_gamemode), start_gamemode, -1)
    end)
end

server.event_handler("disconnect", function()
  
    if server.playercount > 0 or not map_rotation then
        return
    end
    
    if back_to_start and server.gamemode ~= start_gamemode then
        change_map()
    end
end)

server.event_handler("started", function()

    if server.playercount > 0 or not map_rotation then
        return
    end
    
    change_map()
end)

