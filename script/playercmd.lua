
function admincmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_ADMIN) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function mastercmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_MASTER) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end


if server.enable_info_command == 1 then
    dofile("./script/command/info.lua")
end

if server.enable_specall_command == 1 then
    dofile("./script/command/specall.lua")
end

if server.enable_unspecall_command == 1 then
    dofile("./script/command/unspecall.lua")
end

if server.enable_maxclients_command == 1 then
    dofile("./script/command/maxclients.lua")
end

if server.enable_uptime_command == 1 then
    dofile("./script/command/uptime.lua")
end

if server.enable_reload_command == 1 then
    dofile("./script/command/reload.lua")
end

if server.enable_changetime_command == 1 then
    dofile("./script/command/changetime.lua")
end

if server.enable_lpc_command == 1 then
    dofile("./script/command/lpc.lua")
end

if server.enable_players_command == 1 then
    dofile("./script/command/players.lua")
end

if server.enable_names_command == 1 then
    dofile("./script/command/names.lua")
end

if server.enable_votekick_command == 1 then
    dofile("./script/command/votekick.lua")
end

if server.enable_pause_command == 1 then
    dofile("./script/command/pause.lua")
end

if server.enable_resume_command == 1 then
    dofile("./script/command/resume.lua")
end

if server.enable_motd_command == 1 then
    dofile("./script/command/motd.lua")
end

if server.enable_group_command == 1 then
    dofile("./script/command/group.lua")
end

if server.enable_givemaster_command == 1 then
    dofile("./script/command/givemaster.lua")
end

if server.enable_mute_command == 1 then
    dofile("./script/command/mute.lua")
end

if server.enable_unmute_command == 1 then
    dofile("./script/command/unmute.lua")
end

if server.enable_ban_command == 1 then
    dofile("./script/command/ban.lua")
end

if server.enable_kick_command == 1 then
    dofile("./script/command/kick.lua")
end

if server.enable_persist_command == 1 then
    dofile("./script/command/persist.lua")
end

if server.enable_versus_command == 1 then
    dofile("./script/command/versus.lua")
end

if (server.enable_insta_command == 1) or (server.enable_effic_command == 1) then
    dofile("./script/command/insta.lua")
end

if server.enable_1on1_command == 1 then
    dofile("./script/command/1on1.lua")
end

if server.enable_cheater_command == 1 then
    dofile("./script/command/cheater.lua")
end

if server.enable_warning_command == 1 then
    dofile("./script/command/warning.lua")
end

if server.enable_getcn_command == 1 then
    dofile("./script/command/getcn.lua")
end

if server.enable_addbot_command == 1 then
    dofile("./script/command/addbot.lua")
end

if server.enable_admin_command == 1 then
    dofile("./script/command/admin.lua")
end

if server.enable_invadmin_command == 1 then
    dofile("./script/command/invadmin.lua")
end

if server.enable_master_command == 1 then
    dofile("./script/command/master.lua")
end

if server.enable_msg_command == 1 then
    dofile("./script/command/msg.lua")
end

if server.enable_playermsg_command == 1 then
    dofile("./script/command/playermsg.lua")
end

if server.enable_privmsg_command == 1 then
    dofile("./script/command/privmsg.lua")
end
