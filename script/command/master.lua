--[[

	A player command to raise privilege to master

]]


local events = {}

local master_domains = table_unique(server.parse_list(server["master_domains"]))


local function init()

	for i,domain in ipairs(master_domains) do

		events[domain] = auth.add_domain_handler(domain,function(cn,name)

			if server.player_vars(cn).master and server.player_vars(cn).master == domain then

				server.player_vars(cn).master = nil
				server.setmaster(cn)
				server.msg(server.player_displayname(cn) .. " claimed master as '" .. magenta(name) .. "'")
			end

		end)

	end

end


local function unload()

	for i,domain in ipairs(master_domains) do

		if events[domain] then

			auth.cancel_domain_handler(domain,events[domain])
			events[domain] = nil
		end
	end

end


local function run(cn,master_domain)

	if server.player_priv_code(cn) > 0 then

		server.unsetmaster(cn)

	else

		for i,domain in ipairs(master_domains) do

			if master_domain == domain then

				server.player_vars(cn).master = domain
				auth.send_auth_request(cn,domain)
				break
			end
		end
	end

end

return {init = init, run = run, unload = unload}
