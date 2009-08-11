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


if server.enable_1on1_command == 1 then
    dofile("./script/command/1on1.lua")
end
if server.enable_addbot_command == 1 then
    dofile("./script/command/addbot.lua")
end
if server.enable_admin_command == 1 then
    dofile("./script/command/admin.lua")
end
if server.enable_ban_command == 1 then
    dofile("./script/command/ban.lua")
end
if server.enable_changemap_command == 1 then
    if server.enable_forcemap_command == 1 then
	dofile("./script/command/changemap.lua")
    else
	server.log("ERROR: #changemap requires #forcemap - #changemap is not available")
    end
end
if server.enable_changeteam_command == 1 then
    dofile("./script/command/changeteam.lua")
end
if server.enable_changetime_command == 1 then
    dofile("./script/command/changetime.lua")
end
if server.enable_cheater_command == 1 then
    dofile("./script/command/cheater.lua")
end
if server.enable_cw_command == 1 then
    if server.enable_group_command == 1 then
	dofile("./script/command/cw.lua")
    else
	server.log("ERROR: #cw requires #group - #cw is not available")
    end
end
if server.enable_duel_command == 1 then
    if server.enable_suddendeath == 1 then
	dofile("./script/command/duel.lua")
    else
	server.log("ERROR: suddendeath-mode is required - #duel is not available")
    end
end
if server.enable_forcemap_command == 1 then
    dofile("./script/command/forcemap.lua")
end
if server.enable_getcn_command == 1 then
    dofile("./script/command/getcn.lua")
end
if server.enable_givemaster_command == 1 then
    dofile("./script/command/givemaster.lua")
end
if server.enable_group_command == 1 then
    dofile("./script/command/group.lua")
end
if server.enable_info_command == 1 then
    dofile("./script/command/info.lua")
end
if (server.enable_insta_command == 1) or (server.enable_effic_command == 1) then
    if server.enable_suddendeath == 1 then
	dofile("./script/command/insta.lua")
    else
	if server.enable_insta_command == 1 then
	    server.log("ERROR: suddendeath-mode is required - #insta is not available")
	else
	    server.log("ERROR: suddendeath-mode is required - #effic is not available")
	end
    end
end
if server.enable_invadmin_command == 1 then
    dofile("./script/command/invadmin.lua")
end
if server.enable_kick_command == 1 then
    dofile("./script/command/kick.lua")
end
if server.enable_lpc_command == 1 then
    dofile("./script/command/lpc.lua")
end
if server.enable_master_command == 1 then
    dofile("./script/command/master.lua")
end
if server.enable_maxclients_command == 1 then
    dofile("./script/command/maxclients.lua")
    if server.enable_dynamic_maxclients == 1 then
	server.log("WARNING: #maxclients could conflict with resize depending on spectator-count")
    end
    if server.use_resize_mastermode == 1 then
	server.log("WARNING: #maxclients could conflict with resize depending on mastermode")
    end
    if server.use_resize_mode == 1 then
	server.log("WARNING: #maxclients could conflict with resize depending on gamemode")
    end
    if server.enable_resize_command == 1 then
	server.log("WARNING: #maxclients could conflict with #resize")
    end
end
if server.enable_motd_command == 1 then
    dofile("./script/command/motd.lua")
end
if server.enable_msg_command == 1 then
    dofile("./script/command/msg.lua")
end
if server.enable_mute_command == 1 then
    dofile("./script/command/mute.lua")
    if server.enable_unmute_command == 0 then
	server.log("WARNING: #mute available, but not #unmute")
    end
end
if server.enable_names_command == 1 then
    dofile("./script/command/names.lua")
end
if server.enable_pause_command == 1 then
    dofile("./script/command/pause.lua")
    if server.enable_resume_command == 0 then
	server.log("WARNING: #pause available, but not #resume")
    end
end
if server.enable_persist_command == 1 then
    dofile("./script/command/persist.lua")
end
if server.enable_playermsg_command == 1 then
    dofile("./script/command/playermsg.lua")
end
if server.enable_players_command == 1 then
    dofile("./script/command/players.lua")
end
if server.enable_privmsg_command == 1 then
    dofile("./script/command/privmsg.lua")
end
if server.enable_reload_command == 1 then
    dofile("./script/command/reload.lua")
end
if server.enable_resize_command == 1 then
    dofile("./script/command/resize.lua")
    if server.enable_dynamic_maxclients == 1 then
	server.log("WARNING: #resize could conflict with resize depending on spectator-count")
    end
    if server.use_resize_mastermode == 1 then
	server.log("WARNING: #resize could conflict with resize depending on mastermode")
    end
    if server.use_resize_mode == 1 then
	server.log("WARNING: #resize could conflict with resize depending on gamemode")
    end
    if server.enable_maxclients_command == 1 then
	server.log("WARNING: #resize could conflict with #maxclients")
    end
end
if server.enable_resume_command == 1 then
    dofile("./script/command/resume.lua")
    if server.enable_pause_command == 0 then
	server.log("WARNING: #resume available, but not #pause")
    end
end
if server.enable_setmaster_command == 1 then
    dofile("./script/command/setmaster.lua")
end
if server.enable_specall_command == 1 then
    dofile("./script/command/specall.lua")
end
if server.enable_unmute_command == 1 then
    dofile("./script/command/unmute.lua")
    if server.enable_mute_command == 0 then
	server.log("WARNING: #unmute available, but not #mute")
    end
end
if server.enable_unspecall_command == 1 then
    dofile("./script/command/unspecall.lua")
end
if server.enable_uptime_command == 1 then
    dofile("./script/command/uptime.lua")
end
if server.enable_versus_command == 1 then
    if server.enable_suddendeath == 1 then
	dofile("./script/command/versus.lua")
    else
	server.log("ERROR: suddendeath-mode is required - #versus is not available")
    end
end
if server.enable_votekick_command == 1 then
    dofile("./script/command/votekick.lua")
end
if server.enable_warning_command == 1 then
    dofile("./script/command/warning.lua")
end
