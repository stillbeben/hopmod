local force_spec_time = server.force_spec_default_time

local event = {}
local key_function = server.player_iplong
local specs = {}

function server.force_spec(cn, time)

    local key = key_function(cn)

    local spec_time = force_spec_time
    if time then
        spec_time = time * 60 * 1000
    end

    specs[key] = true

    server.spec(cn)
    server.player_msg(cn, "An admin set you to spectator.")
    
    server.sleep(spec_time, function()
        specs[key] = nil
    end)
end

function server.unforce_spec(cn)
    local key = key_function(cn)
    specs[key] = nil
    server.unspec(cn)
end

event.spectator = server.event_handler_object("spectator", function(cn, joined)

    if server.player_priv_code(cn) ~= server.PRIV_NONE then return end
    
    local key = key_function(cn)
    
    if (joined == 0) and specs[key] then
        server.player_msg(cn,red("You cannot leave spectators. It is forced by an admin"))
        server.spec(cn)
    end
end)

local function unload()
    event = {}
end

return {unload = unload}
