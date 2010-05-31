require "crypto"

local function failed_action(cn)
    server.spec(cn)
end

server.event_handler("mapcrc", function(cn, map, crc)

    if map ~= server.map then
        return
    end
    
    local map_info = supported_maps[map]
        
    if not map_info then
        return
    end

    if crypto.tigersum(tostring(crc)) ~= map_info.crc then

        server.msg(string.format("%s is using a modified map", server.player_displayname(cn)))
        server.log(string.format("%s(%i) is using a modified map (crc %s)", server.player_name(cn), cn, crc))
        
        failed_action(cn)
        
        return
    end
end)

