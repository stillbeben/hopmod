-- #master <domain>

local master_domains = table_unique(server.parse_list(server["master_domains"]))

local function master_domainhandler(cn,name)
    server.setmaster(cn)
    server.msg(server.player_name(cn) .. " claimed master as '" .. magenta(name) .. "'")
end

function server.playercmd_master(cn,master_domain)
    if server.player_priv_code(cn) > 0 then
        server.unsetmaster(cn)
    else
	for a,b in ipairs(master_domains) do
	    if master_domain == b then
		server.player_vars(cn).master = b
		auth.sendauthreq(cn,b)
	    end
	end
    end
end

server.event_handler("started",function()
    for a,b in ipairs(master_domains) do
	auth.add_domain_handler(b,function(cn,name)
	    if server.player_vars(cn).master then
        	if server.player_vars(cn).master == b then
		    server.player_vars(cn).master = nil
		    master_domainhandler(cn,name)
		end
	    end
	end)
    end
end)
