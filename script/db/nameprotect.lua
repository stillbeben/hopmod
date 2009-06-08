
local domain_name = server.name_reservation_domain
local domain_id = auth.get_domain_id(domain_name)
if not domain_id then error(string.format("name reservation failure: auth domain '%s' not found",domain_name)) end

local send_verified_msg = function(cn, name)
    server.msg(string.format("%s is verified.", green(name)))
end

auth_domain_handlers[domain_name] = function(cn, name)

    if name ~= server.player_name(cn) then
        server.player_msg(cn, string.format("You authenticated as %s but the server was expecting you to auth as %s", green(name), green(server.player_name(cn))))
        return
    end

    local pvars = server.player_pvars(cn)
    
    pvars.stats_block = false
    
    pvars.name_verified = true
    pvars.reserved_name = name
    pvars.reserved_name_expire = tonumber(server.uptime) + tonumber(server.reserved_name_expire)
    
    send_verified_msg(cn, name)
end

local function isPlayerVerified(cn)
    local pvars = server.player_pvars(cn)
    return pvars.name_verified and pvars.reserved_name == server.player_name(cn) and tonumber(server.uptime) < pvars.reserved_name_expire
end

local function checkPlayerName(cn)

    if tonumber(server.use_name_reservation) == 0 then return end
    
    if auth.found_name(server.player_name(cn), domain_id) and not isPlayerVerified(cn) then
    
        server.sendauthreq(cn, domain_name)
        
        local sid = server.player_sessionid(cn)
        local pid = server.player_id(cn)
        
        server.player_msg(cn, "You are using a reserved name and have about 10 seconds to authenticate your player name.")
        
        server.sleep(13000, function()
            
            if sid ~= server.player_sessionid(cn) or pid ~= server.player_id(cn) then return end
            
            if not isPlayerVerified(cn) then
                server.kick(cn, 0, "server", "using reserved name")
            end
            
        end)
    end
    
end

server.event_handler("active", checkPlayerName)
server.event_handler("rename", checkPlayerName)

function server.playercmd_showauth(cn)
    local pvars = server.player_pvars(cn)
    if pvars.name_verified and pvars.reserved_name == server.player_name(cn) then
        send_verified_msg(cn, server.player_name(cn))
    end
end
