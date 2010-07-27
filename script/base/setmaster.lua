-- Setmaster
-- (c) 2010 Thomas

local failed = { }
local FAILED_LIMIT = 5

local function setmaster(cn, hash, set)
    if not set then
         server.unsetpriv(cn)
        return -1
    end

    local no_hash = hash == ""
    local no_master = server.master == -1
    
    if no_hash and no_master and server.allow_setmaster == 1 then
        server.setmaster(cn)
        return -1
    end
    
    if no_hash or failed[cn] == FAILED_LIMIT then
        return -1 
    end
    
    if server.hashpassword(cn, server.admin_password) == hash then
        if no_master then
            server.setadmin(cn) 
        else
            server.set_invisible_admin(cn)
        end
    else
        server.log(string.format("Player: %s(%i) IP: %s -- failed setmaster login!", server.player_name(cn), cn, server.player_ip(cn)))
        
        failed[cn] = failed[cn] + 1
        
        if failed[cn] == FAILED_LIMIT then
            server.player_msg(cn, red("WARNING: no longer accepting your setmaster requests"))
        end
    end
    
    return -1
end

server.event_handler("connect", function(cn)
    failed[cn] = 0
end)

server.event_handler("disconnect", function(cn)
    failed[cn] = nil
end)

server.event_handler("connect", function(cn)
    failed[cn] = 0
end)

server.event_handler("connecting", function(cn, host, name, hash)
    if hash ~= "" then
	setmaster(cn, hash, 1)
    end
end)

server.event_handler("setmaster", function(cn, hash, set)
    setmaster(cn, hash, set)
end)

