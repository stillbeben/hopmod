--[[

	A player command to raise privilege to admin

]]


local events = {}

local admin_domain = server.admin_domain


local function init()

	events.domain_handler = auth.add_domain_handler(admin_domain, function(cn,name)

		if server.player_vars(cn).admin and server.player_vars(cn).admin == admin_domain then

			server.player_vars(cn).admin = nil
			server.setadmin(cn)
			server.msg(server.player_displayname(cn) .. " claimed admin as '" .. magenta(name) .. "'")
		end

	end)

end


local function unload()

	if events.domain_handler then

		auth.cancel_domain_handler(admin_domain,events.domain_handler)
		events.domain_handler = nil
	end

end


local function run(cn)

    if server.player_priv_code(cn) > 0 then

        server.unsetmaster(cn)

    else

        server.player_vars(cn).admin = admin_domain
        auth.send_auth_request(cn,admin_domain)
    end

end


return {init = init,run = run,unload = unload}
