-- #invadmin

local function invadmin_setInvadmin(cn)

	server.set_invadmin(cn)
	server.player_msg(cn,"(" .. green("Info") .. ") Your rights have been raised to invisible-admin.")

end


server.event_handler("started",function()

	auth.add_domain_handler(server.invadmin_domain,function(cn,name)

		if server.player_vars(cn).invadmin then
			if server.player_vars(cn).invadmin == server.invadmin_domain then
				server.player_vars(cn).invadmin = nil
				invadmin_setInvadmin(cn)
			end
		end

	end)

end)


return function(cn,pw)

	if pw then
		if server.check_admin_password(pw) then
			invadmin_setInvadmin(cn)
		end
	elseif server.player_priv_code(cn) > 0 then
		server.unset_invadmin(cn)
	else
		server.player_vars(cn).invadmin = server.invadmin_domain
		auth.sendauthreq(cn,server.invadmin_domain)
	end

end
