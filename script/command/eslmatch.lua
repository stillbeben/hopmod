-- #insta <map> <cn1> <cn2>
-- #effic <map> <cn1> <cn2>


-- 1on1 (versus) script
-- Version: 0.1
-- (c) 2009 Thomas
-- #versus cn1 cn2 mode map


local running = false

local player1_cn = nil
local player1_id = nil
local player1_ready = false

local player2_cn = nil
local player2_id = nil
local player2_ready = false

local gamecount = 0
local suspended = false

local evthandlers = {}

local function onActive(cn)

    local id = server.player_id(cn)

    if id == player1_id then

        player1_ready = true

    elseif id == player2_id then

        player2_ready = true

    end

    if player1_ready and player2_ready then

        if suspended then
            if  id == match_master then server.setmaster(cn) end
            server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Connected"))

                local countdown = 4

        server.interval(1000, function()
                countdown = countdown - 1

        server.msg(orange(string.format("--[ Resuming the game in %i seconds", countdown)))

        if countdown == 0 then
        suspended = false
        server.pausegame(false)
            return -1
        end

    end)




        else
            server.msg(orange("--[ Game started!"))
            server.changetime(300000)
            server.pausegame(false)
        end

    end
end

local function onMapchange(map, mode)

    gamecount = gamecount + 1

    if gamecount > 1 then
        running = false
        uninstallHandlers()
        return
    end

    server.pausegame(true)
    server.recorddemo("log/demo/" .. player1_name .. "_V_" .. player2_name .. "_" .. mode .. "_" .. map .. ".dmo")

    server.msg(orange("--[ Starting Demo Recording"))
    server.msg(orange("--[ Waiting until all Players loaded the Map."))

end

local function onIntermission()

    if server.player_frags(player1_cn) > server.player_frags(player2_cn) then

        server.msg(green("--[ 1on1 Game ended - (" .. green(server.player_name(player1_cn)) .. ") won the Game!"))

    elseif server.player_frags(player1_cn) < server.player_frags(player2_cn) then

        server.msg(green("--[ 1on1 Game ended - (" .. green(server.player_name(player2_cn)) .. ") won the Game!"))
    else
        server.msg("--[ 1on1 Game ended - No Winner!")
    end

    running = false
    sudden_death = false
    sudden_death_enabled = false
    server.mastermode = 0
    server.unsetmaster()
end


local function onConnect(cn)

    local id = server.player_id(cn)

    if id == player1_id then

        player1_cn = cn
        player1_ready = false
        server.unspec(cn)
    end

    if id == player2_id then

        player2_cn = cn
        player2_ready = false
        server.unspec(cn)
    end

end


local function onDisconnect(cn)

    local id = server.player_id(cn)

    if id == player1_id or id == player2_id then

        if player1_id == id then server.setmaster(player2_cn) else server.setmaster(player1_cn) end -- Handover master

        server.msg(red("--[ Opponent " .. green(server.player_name(cn)) .. " Disconnected Pausing Game"))
        server.pausegame(true)

        suspended = true
    end
end

local function onTimeUpdate(mins)

    if sudden_death_enabled == true then
        if fragged == true then fragged = false; return 0 end

        if mins == 0 then

                for index, cn in ipairs(server.players()) do
                    player_score[index] = server.player_frags(cn)
                end

                if player_score[1] == player_score[2] then
                    server.msg(red("--[ Sudden Death. Next Frag Wins!"))
                    sudden_death = true
                    return 1
                end

        end

        if sudden_death == true then return 1 else return mins end
    else
        return mins
    end
end

local function onFrag()

    if sudden_death_enabled  == true then
        if sudden_death == true then
            sudden_death = false
            fragged = true
            server.changetime(0)
        end
    end

end

local function installHandlers()

    local connect = server.event_handler("connect", onConnect)
    local disconnect = server.event_handler("disconnect", onDisconnect)
    local active = server.event_handler("maploaded", onActive)
    local mapchange = server.event_handler("mapchange", onMapchange)
    local intermission = server.event_handler("intermission", onIntermission)
    local frag = server.event_handler("frag", onFrag)
    local timeupdate = server.event_handler("timeupdate", onTimeUpdate)

    table.insert(evthandlers, connect)
    table.insert(evthandlers, disconnect)
    table.insert(evthandlers, active)
    table.insert(evthandlers, mapchange)
    table.insert(evthandlers, intermission)
    table.insert(evthandlers, frag)
    table.insert(evthandlers, timeupdate)

end

function uninstallHandlers()

    for i,handlerId in ipairs(evthandlers) do server.cancel_handler(handlerId) end
    evthandlers = {}

end

local function ESL(cn, map, player1, player2, mode)

    player1 = tonumber(player1)
    player2 = tonumber(player2)

    if running then
        server.msg(red("--[ Master Cancelled the running versus match"))
        player_score = {}
        sudden_death_enabled = false
        sudden_death = false
        running = false
        gamecount = 0
        server.mastermode = 0
        server.pausegame(false)
        uninstallHandlers()
        server.unsetmaster()
        return
    end

    if not server.valid_cn(player1) or not server.valid_cn(player2) then
        server.player_msg(cn, red("Invalid CN given for the first or second argument."))
        return
    end

    --if player1 == player2 then
    --    server.player_msg(cn, red("player 1 and player 2 have the same CN."))
    --   return
    --end

    player_score = {}
    running = true
    gamecount = 0
    sudden_death_enabled = true
    sudden_death = false
    match_master = server.player_id(cn)
    player1_cn = tonumber(player1)
    player2_cn = tonumber(player2)
    player1_id = server.player_id(player1)
    player2_id = server.player_id(player2)
    player1_name = server.player_name(player1)
    player2_name = server.player_name(player2)

-- FIXME Used for setting master if someone other then player runs script
--    if not cn == player1_cn and not cn == player2_cn then
--        server.setmaster(player1_cn)
--      match_master = server.player_id(player1)
--    else
        server.setmaster(cn); match_master = server.player_id(cn)
--    end

    installHandlers()

    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. blue(server.player_name(player2)) .. " mode: " .. orange(mode)
.. " map: " .. orange(map) ))
    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. blue(server.player_name(player2)) .. " mode: " .. orange(mode)
.. " map: " .. orange(map) ))
    server.msg(green("--[ 1on1 - " .. red(server.player_name(player1)) .. " against " .. blue(server.player_name(player2)) .. " mode: " .. orange(mode)
.. " map: " .. orange(map) ))

    server.specall()
    server.unspec(player1)
    server.unspec(player2)
    server.mastermode = 2
    server.mastermode_owner = -1
    server.pausegame(true)

    local countdown = 6

    server.interval(1000, function()

        countdown = countdown - 1

        server.msg(orange(string.format("-- [ Loading the map in %i seconds", countdown)))

        if countdown == 0 then
            server.changemap(map, mode, -1)
            return -1
        end

    end)
end

return {
    insta_cmd = function(cn, map, player1, player2) ESL(cn, map, player1, player2, "instagib") end, 
    effic_cmd = function(cn, map, player1, player2) ESL(cn, map, player1, player2, "efficiency") end
}
