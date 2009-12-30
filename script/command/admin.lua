--[[

	A player command to raise privilege to admin using auth
    
    WARNING: Enabling the admin command with the admin_domain variable set
    to "" means players with a master key can gain admin privilege on your
    server. TODO Check for this?
]]


local admin_domains = table_unique(server.parse_list(server["admin_domains"])) or ""


local function init() end

local function unload() end

local function run(cn)
    
	if server.player_priv_code(cn) > 0 then

		server.unsetmaster(cn)

	else

		local session_id = server.player_sessionid(cn)

		for i,admin_domain in ipairs(admin_domains) do

			auth.send_request(cn, admin_domain, function(cn, user_id, domain, status)

				if session_id ~= server.player_sessionid(cn) or status ~= auth.request_status.SUCCESS then
					return 
				end
        
				server.setadmin(cn)
        
				server.msg(server.player_displayname(cn) .. " claimed admin as '" .. magenta(user_id) .. "'")
				server.log(string.format("%s playing as %s(%i) used auth to claim admin.", user_id, server.player_name(cn), cn))
	
			end)

		end
    
	end

end


return {init = init,run = run,unload = unload}
