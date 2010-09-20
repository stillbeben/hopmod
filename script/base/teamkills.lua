local LIMIT = 0.25 -- the number of teamkills a player is entitled to make is proportional to their number of frags

local damageEventHandler = nil

local function createDamageHandler()
    assert(damageEventHandler == nil)
    damageEventHandler = server.event_handler("damage", function(target, actor, damage, gun)
        if target ~= actor and server.player_team(actor) == server.player_team(target) then
            if server.player_teamkills(actor) >= (server.player_frags(actor) * LIMIT) then
                return -1
            end
        end
    end)
end

local function cancelDamageHandler()
    assert(damageEventHandler ~= nil)
    server.cancel_handler(damageEventHandler)
    damageEventHandler = nil
end

server.event_handler("mapchange", function()
    
    if gamemodeinfo.teams and (server.mastermode < 2) then
        if not damageEventHandler then
            createDamageHandler()
        end
    else
        if damageEventHandler then
            cancelDamageHandler()
        end
    end
end)

server.event_handler("setmastermode", function()

    server.sleep(500, function()
    
        if gamemodeinfo.teams then
	    if server.mastermode < 2 then
		if not damageEventHandler then
		    createDamageHandler()
		end
	    else
		if damageEventHandler then
		    cancelDamageHandler()
		end
	    end
	end
    end)
end)
