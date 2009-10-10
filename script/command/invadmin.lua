--[[

	A player command to raise privilege to (invisble) admin

]]


local events = {}

local invadmin_domain = server.invadmin_domain


local function set_invadmin(cn)

	server.set_invadmin(cn)
	server.player_msg(cn,"Your rights have been raised to invisible-admin.")

end


local function init()

	events.domain_handler = auth.add_domain_handler(invadmin_domain,function(cn,name)

		if server.player_vars(cn).invadmin and server.player_vars(cn).invadmin == invadmin_domain then

			server.player_vars(cn).invadmin = nil
			set_invadmin(cn)
		end

	end)

end


local function unload()

	if events.domain_handler then

		auth.cancel_domain_handler(invadmin_domain,events.domain_handler)
		events.domain_handler = nil
	end

end


local function run(cn,pw)

	if pw then

		if server.check_admin_password(pw) then

			set_invadmin(cn)
		end

	elseif server.player_priv_code(cn) > 0 then

		server.unset_invadmin(cn)

	else

		server.player_vars(cn).invadmin = invadmin_domain
		auth.send_auth_request(cn,invadmin_domain)
	end

end


return {init = init,run = run,unload = unload}
