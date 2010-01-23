--[[

	A player command to raise privilege to (invisble) admin

	TODO:
		see admin.lua
]]


local invadmin_domains = table_unique(server.parse_list(server["invadmin_domains"])) or ""


local function set_invadmin(cn,name)

	server.set_invisible_admin(cn)
    
	server.player_msg(cn,"Your rights have been raised to invisible-admin.")

	if not name then
		server.log(string.format("%s(%i) claimed (inv)admin.", server.player_name(cn), cn))
	else
		server.log(string.format("%s playing as %s(%i) used auth to claim (inv)admin.", name, server.player_name(cn), cn))
	end

end


local function init() end

local function unload() end


local function run(cn,pw)

	if server.player_priv_code(cn) > 0 then
		server.unsetpriv(cn)

	elseif pw then

		if server.check_admin_password(pw) then

			set_invadmin(cn)
		end

	else

		local session_id = server.player_sessionid(cn)

		for i,invadmin_domain in ipairs(invadmin_domains) do

			auth.send_request(cn, invadmin_domain, function(cn, user_id, domain, status)

				if session_id ~= server.player_sessionid(cn) or status ~= auth.request_status.SUCCESS then
					return
				end

				set_invadmin(cn, user_id)

			end)

		end

	end

end


return {init = init,run = run,unload = unload}
