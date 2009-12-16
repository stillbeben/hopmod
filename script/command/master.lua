
local master_domain = server.master_domain or ""

local function init() end
local function unload() end

local function run(cn)
    
    local session_id = server.player_sessionid(cn)
    
    auth.send_request(cn, master_domain, function(cn, user_id, domain, status)
        
        if session_id ~= server.player_sessionid(cn) then return end
        
        if status ~= auth.request_status.SUCCESS then
            server.player_msg(cn, red("Master command failed: unable to authenticate"))
            return 
        end
        
        server.setmaster(cn)
        
        server.msg(server.player_displayname(cn) .. " claimed master as '" .. magenta(user_id) .. "'")
        server.log(string.format("%s playing as %s(%i) used auth to claim master.", user_id, server.player_name(cn), cn))
    end)
    
end

return {init = init,run = run,unload = unload}
