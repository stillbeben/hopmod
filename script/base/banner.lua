
require "geoip"

local event = {}

local function sendServerBanner(cn)

    if server.player_pvar(cn,"shown_banner") then return end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(1000,function()

        -- cancel if not the same player from 1 second ago
        if sid ~= server.player_sessionid(cn) then return end
        
        server.player_msg(cn, orange(server.servername))
        server.player_msg(cn, server.motd)
        
        server.player_pvar(cn, "shown_banner", true)
    end)
end

local function onConnect(cn)

    local country = geoip.ip_to_country(server.player_ip(cn))
    
    if server.show_country_message == 1 and #country > 0 then
        
        local str_connect = string.format("%s connected from %s.", green(server.player_displayname(cn)), green(country))
        local str_connect_admin = str_connect .. " (IP: " .. red(server.player_ip(cn)) .. ")"
        
        for index, cn in ipairs(server.players()) do
            if server.player_priv_code(cn) == 2 then
                server.player_msg(cn, str_connect_admin)
            else
                server.player_msg(cn, str_connect)
            end
        end
    end    
end

local function onDisconnect(cn)
    server.player_unsetpvar(cn, "shown_banner")
end

event.connect    = server.event_handler_object("connect",onConnect)
event.disconnect = server.event_handler_object("disconnect", onDisconnect)
event.rename     = server.event_handler_object("rename", function(cn) server.player_pvar(cn, "shown_banner", true) end)
event.maploaded  = server.event_handler_object("maploaded", sendServerBanner)

local function unload()
    event = {}
end

return {unload=unload}
