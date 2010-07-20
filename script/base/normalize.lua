
if not server.default_game_on_empty then
    return
end

if not map_rotation then
    error("normalize module depends on the map_rotation object")
    return
end

if not server.valid_gamemode(server.default_gamemode) then
    error(string.format("Error in the normalize module: %s is not a valid gamemode", server.default_gamemode))
    return
end

local DEFAULT_GAMEMODE = server.default_gamemode

function changemap()
    server.changemap(map_rotation.get_map_name(DEFAULT_GAMEMODE), DEFAULT_GAMEMODE)
end

server.event_handler("disconnect", function(cn)
    if server.playercount + server.speccount == 0 and server.gamemode ~= DEFAULT_GAMEMODE then
         changemap()
    end
end)

-- Server started
if not server.mapname then
    changemap()
end

