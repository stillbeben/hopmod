
local domain_name = server.name_reservation_domain
local domain_id = auth.get_domain_id(domain_name)
if not domain_id then error(string.format("name reservation failure: auth domain '%s' not found",domain_name)) end
local is_local = auth.is_domain_local(domain_id)

auth.add_domain_handler(domain_name, function(cn, name)
    
    -- Add user from remote domain. For this situation to arise the same domain
    -- has to be used by another system (i.e. stats auth) to send an auth request.
    if not is_local and not auth.found_name(name, domain_id) then
        auth.add_user(name, "", domain_name)
    end
    
    local pvars = server.player_pvars(cn)
    local vars = server.player_vars(cn)
    
    --if pvars.nameprotect_wanted_authname and pvars.nameprotect_wanted_authname ~= name then
    --    server.player_msg(cn, string.format("You authenticated as %s but the server was expecting you to auth as %s", green(name), green(server.player_name(cn))))
    --    return
    --end
    
    pvars.name_verified = true
    pvars.reserved_name = name
    pvars.reserved_name_expire = tonumber(server.uptime) + tonumber(server.reserved_name_expire)
    
    vars.name_verified = true
    vars.reserved_name = name
    vars.reserved_name_expire = tonumber(server.uptime) + tonumber(server.reserved_name_expire)
    
end)

local function isPlayerVerified(cn)
    local pvars = server.player_pvars(cn)
    return pvars.name_verified and pvars.reserved_name == server.player_name(cn) and tonumber(server.uptime) < pvars.reserved_name_expire
end

local function checkPlayerName(cn)
    
    local playername = server.player_name(cn)

    if auth.found_name(playername, domain_id) and not isPlayerVerified(cn) then
    
        auth.sendauthreq(cn, domain_name)
        
        local sid = server.player_sessionid(cn)
        local pid = server.player_id(cn)
        
        server.player_msg(cn, "You are using a reserved name and have about 10 seconds to authenticate your player name.")
        
        local pvars = server.player_pvars(cn)
        pvars.nameprotect_wanted_authname = playername
        
        server.sleep(13000, function()
            
            if sid ~= server.player_sessionid(cn) or pid ~= server.player_id(cn) then return end
            
            if not isPlayerVerified(cn) then
                server.kick(cn, 0, "server", "using reserved name")
            end
            
        end)
    end
    
end

server.event_handler("active", checkPlayerName)

server.event_handler("rename", function(cn)

    local pvars = server.player_pvars(cn)
    local vars = server.player_vars(cn)
    
    pvars.name_verified = vars.name_verified
    pvars.reserved_name = vars.reserved_name
    pvars.reserved_name_expire = vars.reserved_name_expire

    checkPlayerName(cn)
end)
