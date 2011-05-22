--[[
    A player command to group all players matching pattern <tag>
    
    TODO Improve parameter names
]]

local function group_players(arg1,arg2,arg3)
    
    if not arg1 then
        return -1
    end
    
    local tag
    local team
    
    if arg1 == "all" then
        
        if not arg2 then
            return -1
        end
        
        tag = arg2
        
        if arg3 then
            team = arg3
        else
            team = tag
        end
        
        for spectator in server.gspectators() do
            if string.find(spectator:name(),tag) then
                spectator:unspec()
            end
        end
    else
        tag = arg1
        
        if arg2 then
            team = arg2
        else
            team = tag
        end
    end
    
    for player in server.gplayers() do
        if string.find(player:name(), tag) then
            player:changeteam(team)
        end
    end
end

return function(cn, arg1, arg2, arg3)
    
    if not gamemodeinfo.teams then
        return
    end
    
    if not arg1 then
        return false, "#group [all] <tag> [<team>]"
    end
    
    group_players(arg1, arg2, arg3)
end

