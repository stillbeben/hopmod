--[[
    
    A player command to allow all players to vote to kick other players
    
    Copyright (C) 2009 Thomas
    
    TODO
        * Check required votes have been met when a player disconnects
        * Hide vote message from master if the master is the one being voted for
        * Support command unload
]]

local votes = {}

server.event_handler("connect", function(cn)
    votes[cn] = {}
end)

server.event_handler("disconnect", function(cn, reason)
    votes[cn] = nil
end)

if server.votekick_ad_timer ~= 0 then
    server.interval(server.votekick_ad_timer, function()
        if server.playercount) > 2 and tonumber(server.mastermode) == 0 then
            server.msg("If you see a cheater type: " .. yellow("#votekick \"name\"") .. " or " .. yellow("#votekick cn"))
        end
    end)
end

local function usage()
    return "#votekick <CN> or <NAME>"
end

local function disambiguate_name_list(cn, name)
    
    local message = ""
    
    for i,cn in pairs(server.players()) do
        if name == server.player_name(cn) then
            message = message .. string.format("%i %s\n", cn, name)
        end
    end
    
    server.player_msg(cn, message)
    
end

local function similar_name_list(cn, names)

    local message = ""
    
    for i, player in pairs(names) do
        message = message .. string.format("%i %s\n", player.cn, player.name)
    end
    
    server.player_msg(cn, message)
    
end

return function(cn, kick_who)

    if server.playercount < 3 then
        return false, "There aren't enough players here for votekick to work"
    end
        
    if not kick_who then
        return false, usage()
    end
    
    if not server.valid_cn(kick_who) then
    
        kick_who, info = server.name_to_cn(kick_who)
        
        if not kick_who then
            
            if type(info) == "number" then -- Multiple name matches
                server.player_msg(cn, red(string.format("There are %i players here matching that name:", info)))
                disambiguate_name_list(cn, kick_who)
                return
            elseif -- Similar matches
                
                server.player_msg(cn, red("There are no players found matching that name, but here are some similar names:"))
                similar_name_list(cn, info)
                return
                
            end
            
            return false, usage()
        end
        
    end
    
    if kick_who == cn then
        return false, "You can't vote to kick yourself"
    end
    
    if votes[kick_who][cn] then
        return false, "You have already voted for this player to be kicked"
    end
    
    votes[kick_who][cn] = true
    
    if not votes[kick_who].votes then
        votes[kick_who].votes = 0
    end
    
    votes[kick_who].votes = votes[kick_who].votes + 1
    
    server.msg(green(server.player_displayname(cn)) .. " voted to kick " .. red(server.player_displayname(kick_who)))
    
    local required_votes = round((server.playercount / 2), 0)
    server.msg("Votes: " .. votes[kick_who].votes .. " of " .. required_votes)
    
    if votes[kick_who].votes >= required_votes then
        server.kick(kick_who, 3600, "server", "votekick")
        server.msg("Vote passed to kick player %s", green(server.display_name(kick_who)))
        votes[kick_who] = nil
    end
    
end
