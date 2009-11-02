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

local function get_teamoverflow()
    
    local teamsizes = team_sizes()
    local team_good = teamsizes["good"] or 0 
    local team_evil = teamsizes["evil"] or 0
    
    local overflow = {good = 0, evil = 0}
    
    if not team_good or not team_evil or team_good == team_evil then return overflow,false end
    
    local sizediff = math.abs(team_good - team_evil)
    local correction = math.ceil(sizediff/2)
    
    if team_good > team_evil then
        overflow.good = correction
    else
        overflow.evil = correction
    end
    
    return overflow, correction > 0
end

local function addbots(num)
    
    if num == 0 then return end
    
    for x = 1, num do server.addbot(-1) end
    
    if num == 1 then
        server.msg("The server has added a random skilled bot to balance the teams.")
    else
        server.msg(string.format("The server has added %i random skilled bots to balance the teams.", orange(num)))
    end
end

local function delbots(num)
    for x = 1, num do server.delbot() end
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
    end
    
    local overflow, unbalanced = get_teamoverflow()
    
    if unbalanced then
        if using_bots then
            
            local change = (overflow.good + overflow.evil) - bots_added
            
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

event.disconnect = server.event_handler_object("disconnect",    check_balance)
event.connect = server.event_handler_object("connect",          check_balance)
event.spectator = server.event_handler_object("spectator",      check_balance)
event.reteam = server.event_handler_object("reteam",            check_balance)

event.mapchange = server.event_handler_object("mapchange", function()
    bots_added = 0 -- the server disconnects all the bots at the end of the game
    check_balance() -- for one player case
end)

event.chteamrequest = server.event_handler_object("chteamrequest", function(cn, curteam, newteam)
    local overflow = get_teamoverflow()
    if overflow[newteam] and using_moveblock then
        server.player_msg(cn, red(string.format("Team change disallowed: \"%s\" team has enough players.", newteam)))
        return -1
    end
end)

event.setmastermode = server.event_handler_object("setmastermode", function(cn, current, new)
    if (new ~= "open" and new ~= "veto") and bots_added then
        remove_bots()
        server.player_msg(cn, "Auto Team Balancing has been disabled. It will be re-enabled once the bots have been removed and/or the mastermode has been set to OPEN(0) or VETO(1).")
    end
end)

local function unload_module()
    remove_bots()
    event = nil
end

check_balance() -- in case this module was loaded from #reload

return {unload = unload_module}
