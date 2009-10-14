--[[

	A player command to raise privilege to admin using auth
    
    WARNING: Enabling the admin command with the admin_domain variable set
    to "" means players with a master key can gain admin privilege on your
    server. TODO Check for this?
]]

local admin_domain = server.admin_domain or ""

local function init() end
local function unload() end

local function run(cn)
    
    local session_id = server.player_sessionid(cn)
    
    auth.send_request(cn, admin_domain, function(cn, user_id, domain, status)
        
        if session_id ~= server.player_sessionid(cn) then return end
        
        if status ~= auth.request_status.SUCCESS then
            server.player_msg(cn, red("Admin command failed: unable to authenticate"))
            return 
        end
        
        server.setadmin(cn)
        
        server.msg(server.player_displayname(cn) .. " claimed admin as '" .. magenta(user_id) .. "'")
        server.log(string.format("%s playing as %s(%i) used auth to claim admin.", user_id, server.player_name(cn), cn))
    end)
    
end

return {init = init,run = run,unload = unload}
