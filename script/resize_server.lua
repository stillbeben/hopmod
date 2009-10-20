
local function calcOutput(players, specs)
    return players ^ 2 - players + (players * specs)
end

local output_limit = calcOutput(server.maxplayers, 0)

local function readjustCapacity()

    local sc = server.speccount
    local adminslots = server.reservedslots_occupied
    local pc = server.playercount - sc - adminslots
    local extra = -1
    
    while calcOutput(pc + (extra + 1), sc) <= output_limit do 
        extra = extra + 1
    end
    
    if server.gamemodeinfo.teams then
        extra = extra - ((pc + extra)%2)
    end
    
    server.maxplayers = pc + extra + sc + adminslots
end

local function isOverCapacity()
    return calcOutput(server.playercount, server.speccount) > output_limit
end

server.event_handler("spectator", function(cn, value)

    if tonumber(value) == 1 then
        readjustCapacity()
    else
        if isOverCapacity() then
           server.sleep(1, function()
                server.spec(cn)
                server.player_msg(cn, red("Sorry, there are too many players in the game at the moment, you'll have to wait until someone leaves the game."))
           end)
        else
            readjustCapacity()
        end
    end
end)

server.event_handler("disconnect", function(cn)
    readjustCapacity()
end)
