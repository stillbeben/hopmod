--[[--------------------------------------------------------------------------

    A script to kick spectators who reach the spectate time limit.

    Written by ]Zombie[
--]]--------------------------------------------------------------------------

--
-- helper
--
-- clear memory
local function kickspec_clearmem(kickspec_clearmem_cn)
    for i,cn in ipairs(server.spectators()) do
        if cn == kickspec_clearmem_cn then
            kickspec_spectators[kickspec_clearmem_cn] = nil
            break
        end
    end
end

-- kick and clear mem-place
local function kickspec_kick(kickspec_kick_cn)
    server.kick(kickspec_kick_cn,"1","server","spec-time too high")
    kickspec_clearmem(kickspec_kick_cn)
end

-- spec time check
local function kickspec_checkSpecTimes()
    for i,cn in ipairs(server.spectators()) do
        if kickspec_spectators[cn] > server.kickspec_maxtime then
            kickspec_kick(cn)
        end
    end
end

--
-- events
--
-- notice new spectators, remove leaving persons
local function kickspec_spectator_event(kickspec_spec_cn,kickspec_spec_joined)
    if kickspec_spec_joined == 1 then
        kickspec_spectators[kickspec_spec_cn] = 0
    elseif kickspec_spec_joined == 0 then
        kickspec_clearmem(kickspec_spec_cn)
    end
end

-- maptime - timeplayed = return [in seconds]
local function kickspec_spectime(kickspec_spectime_maptime,kickspec_spectime_cn)
    return (kickspec_spectime_maptime - (server.player_timeplayed(kickspec_spectime_cn) * 1000))
end

-- sum the spectimes on "intermission"
local function kickspec_finishedgame_event()
    lgamelimit = tonumber(server.gamelimit)
    for i,cn in ipairs(server.spectators()) do
        kickspec_spectators[cn] = kickspec_spectators[cn] + kickspec_spectime(lgamelimit,cn)
    end
end

--
-- enable/ disable events
--
function kickspec_enabler()
    kickspec_spectators = {}
    for i,cn in ipairs(server.spectators()) do
        kickspec_spectators[cn] = 0
    end

    local kickspec_disconnect = server.event_handler("disconnect",kickspec_clearmem)
    local kickspec_spectator = server.event_handler("spectator",kickspec_spectator_event)
    local kickspec_finishedgame = server.event_handler("finishedgame",kickspec_finishedgame_event)

    kickspec_events = {}
    table.insert(kickspec_events,kickspec_disconnect)
    table.insert(kickspec_events,kickspec_spectator)
    table.insert(kickspec_events,kickspec_finishedgame)

    kickspec_check_enabled = 1
end

function kickspec_disabler()
    kickspec_check_enabled = 0

    for i,handlerId in ipairs(kickspec_events) do
        server.cancel_handler(handlerId)
    end
    kickspec_events = {}

    kickspec_spectators = {}
end

--
-- always running events
--
-- disable kickspec, when mastermode is not open
server.event_handler("setmastermode",function(old,new)
    if not ( new == "open" ) then
        kickspec_disabler()
    else
        kickspec_enabler()
    end
end)

--
-- on start
--
server.event_handler("started",function()
    kickspec_enabler()

    kickspec_check_enabled = 1
    -- check spec times every 5 minutes
    server.interval(300000,function()
        if kickspec_check_enabled == 1 then
            kickspec_checkSpecTimes()
        end
    end)
end) 
