--[[
    Auto Team Balancing Using Bots
    
]]

local event = {}
local using_bots = server.teambalance_using_bots
local using_moveblock = server.teambalance_using_moveblock
local bots_added = 0

local function team_sizes()
    
    local teams = {}
    local greatest_teamsize = 0
    
    local function increment(t, key)
        local newvalue = (t[key] or 0) + 1
        t[key] = newvalue
        return newvalue
    end
    
    for player in server.gplayers() do
        greatest_teamsize = math.max(greatest_teamsize, increment(teams, player:team()))
    end
    
    -- Discounting bot players
    
    return teams, greatest_teamsize
end

local function addbots(num)
    
    if num == 0 then return end
    
    for x = 1, num do server.addbot(-1) end
end

local function delbots(num)
    local bots = server.bots()
    local num = math.min(num, #bots)
    for x = 1, num do server.delbot(bots[x]) end
end

local function remove_bots()
    if bots_added > 0 then
        delbots(bots_added)
        bots_added = 0
    end
end

local function check_balance()

    if not gamemodeinfo.teams or server.mastermode > 1 then -- deactivate conditions
        remove_bots()
        return
    end
    
    local teams = team_sizes()
    local size_difference = math.abs((teams.good or 0) - (teams.evil or 0))
    local unbalanced = size_difference > 0
    
    if unbalanced then
        if using_bots then
            
            local change = size_difference - bots_added
        
            if change ~= 0 then
                
                local change_function = addbots
                if change < 0 then change_function = delbots end
                change_function(math.abs(change))
                
                bots_added = bots_added + change
            end
        end
    else
        remove_bots()
    end
    
end

server.event_handler("disconnect",  check_balance)
server.event_handler("connect",     check_balance)
server.event_handler("spectator",   check_balance)
server.event_handler("reteam",      check_balance)

server.event_handler("mapchange", function()
    bots_added = 0 -- the server disconnects all the bots at the end of the game
    check_balance() -- for one player case
end)

server.event_handler("chteamrequest", function(cn, curteam, newteam)
    local teams = team_sizes()
    if (teams[curteam] or 0) > (teams[newteam] or 0) and using_moveblock then
        server.player_msg(cn, red(string.format("Team change disallowed: \"%s\" team has enough players.", newteam)))
        return -1
    end
end)

server.event_handler("setmastermode", function(cn, current, new)
    if (new ~= "open" and new ~= "veto") and bots_added then
        remove_bots()
        server.player_msg(cn, "Auto Team Balancing has been disabled. It will be re-enabled once the bots have been removed and/or the mastermode has been set to OPEN(0) or VETO(1).")
    end
end)

local function unload_module()
    remove_bots()
end

check_balance() -- in case this module was loaded from #reload

return {unload = unload_module}
