# Default values for external variables

global motd "Running Hopmod"

global banlist_file "conf/bans"
global geoip_db_file "share/GeoIP.dat"

global use_script_socket_server 1
global script_socket_port 7894
global script_socket_password ""

global allow_mapvote 1
global allow_modevote 1

global auth_db_filename "log/auth.sqlite"

global record_player_stats 1
global stats_db_filename "log/stats.sqlite"
global stats_use_auth 0
global stats_auth_domain ""
global stats_debug 0
global stats_use_sqlite 1
global stats_use_json 0

global use_server_maprotation 1

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

global use_best_map_size 0
global small_gamesize 5

global first_map "complex"
global first_gamemode "ffa"

global use_irc_bot 0

flood_protect_text 1000
flood_protect_sayteam 1000
flood_protect_mapvote 1000
flood_protect_switchteam 1000
flood_protect_switchname 1000

global teamkill_limit 7
global teamkill_showlimit 1

global enable_ping_limiter 1
global ping_limiter_tick 25000
global ping_limit 500
global lag_limit 30
global ping_limit_warnings 4

global enable_cheater_command 0
global enable_votekick_command 0

global enable_dynamic_maxclients 0

global disable_masterauth_in_coopedit 0
