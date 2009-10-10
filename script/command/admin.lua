--[[

	A player command to raise privilege to admin using auth
]]

local admin_domain = server.admin_domain or ""

local function init() end
local function unload() end

local function run(cn)

    auth.send_request(cn, admin_domain, function(cn, user_id, domain, status)
        
        if status ~= auth.request_status.SUCCESS then return end
    
        server.setadmin(cn)
        server.msg(server.player_displayname(cn) .. " claimed admin as '" .. magenta(user_id) .. "'")
        
    end)
    
end

return {init = init,run = run,unload = unload}
