-- Setmaster
-- (c) 2010 Thomas


local setmaster_abuse = { }

server.event_handler("connect", function(cn)
    setmaster_abuse[cn] = 0
end)

server.event_handler("disconnect", function(cn)
    setmaster_abuse[cn] = nil
end)


server.event_handler("setmaster", function(cn, hash)

    if (hash == "" and server.player_priv(cn) ~= "none") or server.player_priv(cn) ~= "none" then
        server.unsetpriv(cn) 
        return -1
    end

    local admin_count = 0
    local master_count = 0

    for _, cn in ipairs(server.clients()) do 
        local priv = server.player_priv(cn)
        if priv == "master" then master_count = master_count + 1 end
        if priv == "admin" then admin_count = admin_count + 1 end
    end

    if hash == "" and server.allow_setmaster and master_count == 0 and admin_count == 0 then
        server.setmaster(cn)
        return -1
    end

    if hash == "" then 
        return -1 
    end

    if server.hashpassword(cn, server.admin_password) == hash then
        if admin_count == 0 then
            server.setadmin(cn) 
        else
            server.set_invisible_admin(cn)
        end
    else
        server.log(string.format("Player: %s(%i) IP: %s -- tried to use setmaster with a wrong password.", server.player_name(cn), cn, server.player_ip(cn)))
        setmaster_abuse[cn] = setmaster_abuse[cn] + 1
        if setmaster_abuse[cn] >= 2 then
            server.kick(cn, 300, "", red() .. "for trying to bruteforce the admin password") 
            return -1
        end
		server.player_msg(cn, red() .. "WARNING: " .. white() .. "Please don't try to bruteforce the admin password!\nYou will be" .. red() .. " kicked " .. white() .. "after next failed attempt!")
    end
    
    return -1
end)
