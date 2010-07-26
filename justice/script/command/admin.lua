--[[

	A player command to raise privilege to admin using auth
    
    WARNING: Enabling the admin command with the admin_domain variable set
    to "" means players with a master key can gain admin privilege on your
    server. TODO Check for this?
]]

local admin_domains = table_unique(server.parse_list(server.admin_domains))

return function(cn)

    local session_id = server.player_sessionid(cn)
    
    for _, admin_domain in ipairs(admin_domains) do

        auth.send_request(cn, admin_domain, function(cn, user_id, domain, status)

            if session_id ~= server.player_sessionid(cn) or status ~= auth.request_status.SUCCESS then
                server.player_msg(cn, red("Admin command failed: unable to authenticate"))
                return 
            end
    
            server.setadmin(cn)
    
            server.msg(server.player_displayname(cn) .. " claimed admin as '" .. magenta(user_id) .. "'")
            server.log(string.format("%s playing as %s(%i) used auth to claim admin.", user_id, server.player_name(cn), cn))
            
            return
        end)
    end
end
