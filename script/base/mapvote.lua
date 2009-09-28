--[[
    
    Base module for restricting map votes
    
    Copyright (C) 2009 Graham Daws
    
    MAINTAINER
        Graham
    
    GUIDELINES
        * Transfer global configuration variables to local scope variables
]]

local deny_mapvote = (server.allow_mapvote == 0)
local deny_modevote = (server.allow_modevote == 0)
local deny_coopedit = (server.disallow_coopedit == 1)
local deny_unknown_map = (server.mapvote_disallow_unknown_map == 1)
local allowed_modes = table_unique(server.parse_list(server["allowed_gamemodes"]))

local function mapvote(cn, map, mode)

    -- Admin players are free from voting restrictions
    if server.player_priv_code(cn) == server.ADMIN then
        return
    end
    
    if deny_mapvote then
    
        server.player_msg(cn, red("Map voting is disabled."))
        return -1
        
	end
    
    -- Find voted mode in the allowed game modes list
    local isFound = false
    
    for i, gamemode in pairs(allowed_modes) do
        if mode == gamemode then 
            isFound = true
            break
        end
    end
    
    if isFound == false then
        server.player_msg(cn, red("This server doesn't accept votes for '" .. mode .. "' game mode."))
        return -1
    end

    if deny_unknown_map and mode ~= "coop edit" then
    
        local globalFound = server.is_known_map(map)
        local localFound = server.is_known_map(map, mode)
        
        if not globalFound then
            server.player_msg(cn, red("\"" .. map .. "\" is an unknown map."))
            return -1
        end
        
        if not localFound then
            server.player_msg(cn, red("\"" .. map .. "\" is not a map you can play in " .. mode .. "."))
            return -1
        end
    end
    
end

server.event_handler("mapvote", mapvote)
