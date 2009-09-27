
function server.mute(cn)
    server.player_pvar(cn, "mute", true)
end

function server.unmute(cn)
    server.player_unsetpvar(cn, "mute")
end

local function onText(cn,text)
    
    local block = 0
    
    if block == 0 and server.player_pvar(cn,"mute") then
        server.player_msg(cn, red() .. "Your chat messages are being blocked.")
        block = -1
    end
    
    return block
end

server.event_handler("text", onText)
server.event_handler("sayteam", onText)
