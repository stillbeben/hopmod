# Default values for external variables

global motd "Running Hopmod"

global banlist_file "conf/bans"
global geoip_db_file "share/GeoIP.dat"

global use_script_socket_server 1
global script_socket_port 7894

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

global ffa_maps [metl4 deathtek fanatic_quake aard3c metl2 ruby curvedm metl3
    nmp8 complex douze killfactory lostinspace oasis aqueducts corruption thor
    academy tartech refuge kffa kalking1 orbe wake5 ot fragplaza hog2 
    roughinery shadowed DM_BS1 shindou moonlite darkdeath thetowers
    konkuri-to stemple tejen ogrosupply frostbyte fanatic_castle_trap
    nmp10 island neondevastation neonpanic orion katrez_d ksauer1 
    pgdm oddworld phosgene sdm1 kmap5 serpentine battleofthenile
    guacamole hades paradigm mechanic wdcd]

global insta_maps [fanatic_quake aard3c metl2 curvedm metl3 nmp8 complex douze
    academy refuge kffa kalking1 ot island neondevastation neonpanic orion 
    oddworld sdm1]

global capture_maps [river_c paradigm fb_capture urban_c serenity nevil_c
    lostinspace face-capture nmp9 c_valley nmp4 nmp8 fc3 ph-capture 
    monastery corruption hades asteroids venice relic frostbyte ogrosupply
    hallo reissen akroseum duomo capture_night c_egypt tejen fc4 dust2 campo
    killcore3 damnation]

global ctf_maps [hallo reissen dust2 berlin_wall shipwreck akroseum flagstone
    face-capture valhalla urban_c damnation mach2 redemption tejen europium 
    capture_night l_ctf forge sctf1 paradiselost campo wdcd]

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

global small_maps [aard3c metl2 metl3 nmp8 complex douze academy refuge shinmei1 kffa kalking1 ot neondevastation neonpanic dock island oddworld sdm1]
global big_maps [deathtek fanatic_quake ruby cuvedm killfactory lostinspace aqueducts corruption orbe shadowed konkuri-to darkdeath ksauer1 pgdm thetowers wdcd]
global use_best_map_size 0
global small_gamesize 5

global first_map "metl4"
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
global ping_limit_warnings 4

global enable_cheater_command 0
