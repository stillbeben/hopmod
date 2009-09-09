-- #admin

local function admin_domainhandler(cn,name)
    server.setadmin(cn)
    server.msg(server.player_name(cn) .. " claimed admin as '" .. magenta(name) .. "'")
end

server.event_handler("started", function()
    auth.add_domain_handler(server.admin_domain, function(cn,name)
        if server.player_vars(cn).admin then
            if server.player_vars(cn).admin == server.admin_domain then
                server.player_vars(cn).admin = nil
                admin_domainhandler(cn,name)
            end
        end
    end)
end)

return function(cn)

    if server.player_priv_code(cn) > 0 then
        server.unsetmaster(cn)
    else
        server.player_vars(cn).admin = server.admin_domain
        auth.sendauthreq(cn,server.admin_domain)
    end
    
end
