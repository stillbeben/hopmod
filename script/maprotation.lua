
local gamecount = 0
local maps = {}

function server.reload_maprotation()

    local parse = server.parse_list
    
    maps["ffa"] = table_unique(parse(server["ffa_maps"]))
    maps["templay"] = table_unique(parse(server["teamplay_maps"]))
    maps["efficiency"] = table_unique(parse(server["efficiency_maps"]))
    maps["efficiency team"] = table_unique(parse(server["efficiency team_maps"]))
    maps["tactics"] = table_unique(parse(server["tactics_maps"]))
    maps["tactics team"] = table_unique(parse(server["tactics team_maps"]))
    maps["instagib"] = table_unique(parse(server["instagib_maps"]))
    maps["instagib team"] = table_unique(parse(server["instagib team_maps"]))
    maps["capture"] = table_unique(parse(server["capture_maps"]))
    maps["regen capture"] = table_unique(parse(server["regen capture_maps"]))
    maps["ctf"] = table_unique(parse(server["ctf_maps"]))
    maps["insta ctf"] = table_unique(parse(server["insta ctf_maps"]))
    maps["protect"] = table_unique(parse(server["protect_maps"]))
    maps["insta protect"] = table_unique(parse(server["insta protect_maps"]))
    
end

local function nextmap(gmode,i)
    maplist = maps[gmode]
    if not maplist then return nil end
    local nm = maplist[(i % #maplist)+1]
    if nm == server.map then return nextmap(gmode,i+1) end
    return nm
end

server.event_handler("setnextgame",function()
    
    local use_first_map = tonumber(server.use_first_map)
    
    if tonumber(server.playercount) == 0 and use_first_map then
        server.next_mode = server.first_mode
        server.next_map = server.first_map
    else
        if tonumber(server.use_server_maprotation) == 1 then
            server.next_mode = server.gamemode
            local nm = nextmap(server.gamemode, gamecount)
            if nm then server.next_map = nm end
        end
    end
    
    gamecount = gamecount + 1
    
end)

function server.playercmd_nextmap(cn)
    
    if tonumber(server.use_server_maprotation) == 1 then
        local nm = nextmap(server.gamemode, gamecount)
        if nm then
            server.player_msg(cn, "The next map is " .. green(nm) .. ".")
            return
        end
    end
    
    -- TODO keep an official map rotation list
    server.player_msg(cn, "?")
end

server.reload_maprotation()
