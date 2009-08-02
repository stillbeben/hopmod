# Default values for external variables

global motd "Running Hopmod"

global banlist_file "conf/bans"
global geoip_db_file "share/GeoIP.dat"

global use_script_socket_server 1
global script_socket_port 7894
global script_socket_password ""
global shell_label "server"

global allow_mapvote 1
global allow_modevote 1
global mapvote_disallow_unknown_map 0

global auth_db_filename "log/auth.sqlite"
global auth_use_sqlite 1

global record_player_stats 1
global stats_db_filename "log/stats.sqlite"
global stats_use_auth 0
global stats_auth_domain ""
global stats_tell_auth_name 0
global stats_debug 0
global stats_use_sqlite 1
global stats_use_json 0
global stats_overwrite_name_with_authname 0
global stats_record_only_authnames 0
global enable_stats_command 0
global exp_stats_total_kpd 0
global exp_stats_total_acc 0
global exp_stats_total_rank 0

global use_name_reservation 0
global enable_auth_command 0
global enable_showauth_command 0
global enable_shownonauth_command 0
global name_reservation_domain ""
global reserved_name_expire (mins 3600)
global use_tag_reservation 0

global use_server_maprotation 1
global enable_nextmap_command 1

global ffa_maps [complex douze ot academy metl2 metl3 nmp8 refuge tartech
    kalking1 dock turbine fanatic_quake oddworld wake5 aard3c curvedm
    fragplaza pgdm kffa neondevastation hog2 memento neonpanic lostinspace
    DM_BS1 shindou sdm1 shinmei1 stemple powerplant phosgene oasis island
    metl4 ruby frozen ksauer1 killfactory corruption deathtek aqueducts orbe]

global insta_maps [complex douze ot academy metl2 metl3 nmp8 tartech kalking1
    dock turbine fanatic_quake oddworld wake5 aard3c curvedm kffa
    neondevastation hog2 neonpanic sdm1]

global capture_maps [urban_c nevil_c fb_capture nmp9 c_valley lostinspace fc3
    face-capture nmp4 nmp8 hallo monastery ph-capture hades fc4 relic frostbyte
    venice river_c paradigm corruption asteroids ogrosupply reissen akroseum
    duomo capture_night c_egypt tejen dust2 campo killcore3 damnation serenity
    cwcastle]

global ctf_maps [hallo reissen face-capture flagstone shipwreck urban_c dust2
    berlin_wall akroseum valhalla damnation mach2 redemption tejen europium
    capture_night l_ctf forge campo wdcd sacrifice core_transfer recovery]

global "teamplay_maps" &ffa_maps
global "efficiency_maps" &ffa_maps
global "efficiency team_maps" &ffa_maps
global "tactics_maps" &ffa_maps
global "tactics team_maps" &ffa_maps
global "instagib_maps" &insta_maps
global "instagib team_maps" &insta_maps
global "regen capture_maps" &capture_maps
global "insta ctf_maps" &ctf_maps
global "protect_maps" &ctf_maps
global "insta protect_maps" &ctf_maps

global small_maps [complex douze ot academy metl2 metl3 nmp8 tartech dock
    turbine oddworld aard3c kffa neondevastation hog2 memento neonpanic
    sdm1 shinmei1 island metl4 frozen]

global big_maps [refuge kalking1 fanatic_quake lostinspace DM_BS1 shindou
    stemple powerplant killfactory corruption deathtek aqueducts orbe]

global game_modes [instagib efficiency tactics ffa
    "instagib team" "efficiency team" "tactics team" teamplay
    "insta ctf" ctf "insta protect" protect capture "regen capture"]

global use_best_map_size 0
global small_gamesize 5

global first_map "complex"
global first_gamemode "ffa"
global firstgame_on_empty 0

global use_irc_bot 0

flood_protect_text 1000
flood_protect_sayteam 1000
flood_protect_mapvote 1000
flood_protect_switchteam 10000
flood_protect_switchname 1000
flood_protect_remip 10000
flood_protect_newmap 10000

global teamkill_limit 7
global teamkill_showlimit 1

global enable_ping_limiter 1
global ping_limiter_tick 25000
global ping_limit 500
global lag_limit 30
global ping_limit_warnings 4

global enable_dynamic_maxclients 0

global disable_masterauth_in_coopedit 0

global use_kickspec 0
global kickspec_maxtime (mins 30)

global enable_ownage_messages 0

global enable_suddendeath 1
global enable_sd_command 1
global enable_nosd_command 1

global change_default_maptime 0
global default_maptime (mins 10)

global change_default_mastermode 0
global default_mastermode 2
global disallow_only_private_mastermode 0
global allow_only_public_mastermodes 0
global allow_only_nonpublic_mastermodes 0

global use_change_mode_when_empty 0

global use_modmap 1
global enable_unfspec_command 1

global use_live_server_stats 1
global use_live_server_single_stats 0

global resize_totalmaxplayers 7
global resize_totalminplayers 1
global use_resize_mastermode 1
global resize_mastermode locked

global use_teambalncer 0
global use_teambalancer_bot 0
global use_teambalancer_xonx 0



global enable_cheater_command 0
global cheater_ad_timer (mins 3)

global enable_votekick_command 0
global votekick_ad_timer (secs 311)

global enable_eval_command 0

global enable_reload_command 0

global enable_1on1_command 0
global enable_duel_command 0
global enable_insta_command 0
global enable_versus_command 0

global enable_addbot_command 0

global enable_setmaster_command 0
global enable_givemaster_command 0
global master_domains []
global enable_master_command 0
global admin_domain ""
global enable_admin_command 0
global invadmin_domain ""
global enable_invadmin_command 0

global enable_kick_command 0
global enable_ban_command 0

global enable_forcemap_command 0
global enable_changemap_command 0

global enable_changeteam_command 0
global enable_group_command 0

global enable_changetime_command 0

global enable_cw_command 0

global enable_getcn_command 0

global enable_info_command 0

global enable_lpc_command 0

global enable_maxclients_command 0
global enable_resize_command 0

global enable_motd_command 0

global enable_msg_command 0
global enable_warning_command 0

global enable_mute_command 0
global enable_unmute_command 0

global enable_names_command 0

global enable_pause_command 0
global enable_resume_command 0

global enable_persist_command 0

global enable_playermsg_command 0
global enable_privmsg_command 0

global enable_players_command 0

global enable_specall_command 0
global enable_unspecall_command 0

global enable_uptime_command 0
