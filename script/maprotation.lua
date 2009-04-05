
local gamecount = 0
local maps = {}

function server.reload_maprotation()

    maps["ffa"] = server.parse_list(server["ffa_maps"])
    maps["templay"] = server.parse_list(server["teamplay_maps"])
    maps["efficiency"] = server.parse_list(server["efficiency_maps"])
    maps["efficiency team"] = server.parse_list(server["efficiency team_maps"])
    maps["tactics"] = server.parse_list(server["tactics_maps"])
    maps["tactics team"] = server.parse_list(server["tactics team_maps"])
    maps["instagib"] = server.parse_list(server["instagib_maps"])
    maps["instagib team"] = server.parse_list(server["instagib team_maps"])
    maps["capture"] = server.parse_list(server["capture_maps"])
    maps["regen capture"] = server.parse_list(server["regen capture_maps"])
    maps["ctf"] = server.parse_list(server["ctf_maps"])
    maps["insta ctf"] = server.parse_list(server["insta ctf_maps"])
    maps["protect"] = server.parse_list(server["protect_maps"])
    maps["insta protect"] = server.parse_list(server["insta protect_maps"])
    
end

server.event_handler("setnextgame",function()
    
    gamecount = gamecount + 1
    
    if tonumber(server.use_server_maprotation) == 1 then
        
        server.next_mode = server.gamemode
        
        maplist = maps[server.gamemode]
        if maplist then
            server.next_map = maplist[(gamecount % #maplist)+1]
        end
    end
    
end)

server.reload_maprotation()
