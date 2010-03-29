local VERY_LONG_BANTIME = 63072000
local SHORT_BANTIME = 1440

local kick_signal = server.create_event_signal("kick")
local temporary_bans = {}

local function kick_banned_players(ipmask, bantime, admin, reason)

    for _, cn in ipairs(server.clients()) do
        if net.ipmask(server.player_iplong(cn)) == net.ipmask(ipmask) then
            server.disconnect(cn, server.DISC_KICK, reason)
            kick_signal(cn, bantime, admin or "", reason or "")
        end
    end
end

function server.kick(cn, bantime, admin, reason)
    
    if not bantime then 
        bantime = SHORT_BANTIME
    else
        if bantime == -1 then
            bantime = VERY_LONG_BANTIME
        end
    end
    
    reason = reason or ""
    
    local address = server.player_iplong(cn)

    server.set_ip_var(address, "ban_expire", os.time() + bantime)
    server.set_ip_var(address, "ban_admin", admin)
    server.set_ip_var(address, "ban_reason", reason)
    server.set_ip_var(address, "ban_time", os.date())

    kick_banned_players(address, bantime, admin, reason)
end

function server.ban(ipmask, bantime, reason)

    if not bantime or bantime == -1 then
        bantime = VERY_LONG_BANTIME
    end
    
    reason = reason or ""
    
    server.set_ip_var(ipmask, "ban_expire", os.time() + bantime)
    server.set_ip_var(ipmask, "ban_reason", reason)
    
    if not server.ip_vars(ipmask).ban_time then
        server.set_ip_var(ipmask, "ban_time", os.date())
    end
    
    kick_banned_players(ipmask, bantime, admin, reason)
end

function server.unban(ipmask)
    server.set_ip_var(ipmask, "ban_expire", nil)
    server.set_ip_var(ipmask, "ban_admin", nil)
    server.set_ip_var(ipmask, "ban_reason", nil)
    server.set_ip_var(ipmask, "ban_time", nil)
end

server.event_handler("connecting", function(cn, hostname, name, password)
    local bantime = server.ip_vars(hostname).ban_expire
    if bantime then
        if bantime > os.time() then
            return -1
        else
            server.unban(hostname)
        end
    end
end)

server.event_handler("clearbans_request", function()
    
    for _, iplong in pairs(temporary_bans) do
        server.unban(iplong)
    end
    
    temporary_bans = {}
    
    server.msg("Cleared all bans")
end)

server.event_handler("kick_request", function(admin_cn, admin_name, target, bantime, reason)

    if bantime <= SHORT_BANTIME then
        temporary_bans[#temporary_bans] = server.player_iplong(target)
    end
    
    server.kick(target, bantime, admin_name, reason)
end)
