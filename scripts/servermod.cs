
exec "scripts/irc.csl"
exec "scripts/playercommands.cs"

logfile "logs/server.log" server_log
log = [
    reference text arg1
    if $log_timestamp [server_log (format "[%1] %2" (time (now)) $text)] [server_log $text]
]

event_handler $onconnect [
    reference cn arg1
    
    local country_log ""
    if (symbol? country) [country_log = (format "(%1)" (country (player_ip $cn)))]
    
    log (format "%1(%2)(%3)%4 connected" (player_name $cn) $cn (player_ip $cn) $country_log)
    
    if (symbol? country) [
        local location (country (player_ip $cn))
        if (strcmp $location "unknown") [] [
            msg (format "%1 is connected from %2" (green (player_name $cn)) (green $location))
        ]
    ]
    
    sleep (secs 2) [
        local cn @cn
        local connection_id @(player_conid $cn)
        if (= $connection_id (player_conid $cn)) [
            privmsg $cn (orange [@servername server])
            privmsg $cn $motd
        ]
    ]
    
    if (&& (&& (symbol? showaliases) $enable_showaliases) (listlen (privplayers))) [
        local altnames (try showaliases $cn [""])
        foreach (privplayers) [
            privmsg $arg1 (format "Past names used by %1: %2" (green (player_name $cn)) (green $altnames))
        ]
    ]
    
    if (strcmp $gamemode "coopedit") [
        privconsole $cn script "Warning: running /newmap command as an unprivileged player will get you kicked and banned."
    ]
]

event_handler $ondisconnect [
    parameters cn reason
    
    discmsg = "disconnected"
    if (! (= $reason 0)) [discmsg = (format "disconnected (%1)(%2)" (disc_reason $reason) (player_ip $cn))]
    log (format "%1(%2) %3, connection time: %4" (player_name $cn) $cn $discmsg (duration (player_contime $cn)))
    
    if (= $reason $DISC_KICK) [player_var $cn kicked 1]
    if (= $playercount 0) [sched defaultgame]
]

event_handler $onrename [
    parameters cn oldname newname
    
    log (format "%1(%2) renamed to %3" $oldname $cn $newname)
]

event_handler $onreteam [
    parameters cn oldteam newteam
    
    log (format "%1(%2) changed team to %3" (player_name $cn) $cn $newteam)
]

event_handler $onchmm [
    parameters cn newmode
    
    log (format "mastermode is now %1, was set by %2(%3)" $newmode (player_name $cn) $cn)
]

event_handler $onkick [
    parameters target master
    target_id = (format "%1(%2)" (player_name $target) $target)
    if (= $master -1) [master_id = "a server admin"] [master_id = (format "%1(%2)" (player_name $master) $master)]
    log (format "%1 was kicked and banned by %2" $target_id $master_id)
]

event_handler $ontext [
    parameters cn text
    mutetag = ""
    
    if (! $allow_talk) [
        privmsg $cn (err (concat "Talking is banned at this time because:" $disallow_talk_reason))
        veto 1
    ]
    
    if (player_var $cn mute) [
        mutetag = "(muted)"
        privmsg $cn (err "Your voice privilege has been revoked.")
        veto 1
    ]
    
    local cmd 0
    
    if (match "^#.*$" $text) [
        veto 1
        arguments = (split $text " ")
        
        cmdname = (at (split (at $arguments 0) #) 0)
        filename = (format "./scripts/commands/%1.csl" $cmdname)
        if (path? $filename) [exec $filename] [
            dynamic_command = (concatword playercmd_ $cmdname)
            if (symbol? $dynamic_command) [
                arguments = (erase1st $arguments)
                try do [@dynamic_command @arguments] [
                    log_error [@dynamic_command function failed with error @arg1]
                    privmsg $cn (err "Command failed. Check your arguments and your privilege level.")
                ]
            ] [privmsg $cn (err "Command not found.")]
        ]
        cmd = 1
    ]
    
    if (&& (! $cmd) (strcmp (player_status $cn) "spying")) [
        veto 1
        console (player_name $cn) $text
    ]
    
    log (format "%1(%2)%3: %4" (player_name $cn) $cn $mutetag $text)
]

event_handler $onsayteam [
    parameters cn text
    log (format "%1(%2)(team): %3" (player_name $cn) $cn $text)
]
    
event_handler $onmapvote [
    parameters cn mode map
    
    local admin (strcmp (player_priv $cn) "admin")
    
    if (|| $admin $mapvoting_enabled) [
        log (format "%1(%2) suggests %3 on map %4" (player_name $cn) $cn $mode $map)
    ] [
        privmsg $cn (err "Map voting is disabled.")
        veto 1
    ]
]

event_handler $onsetmap [
    if $custom_maprotation_enabled setnextmap
    if $custom_gametime_enabled [gametime $custom_gametime]
]

event_handler $onmapchanged [

    if (> $playercount 0) [
        log (format "new game: %1" (gameinfo))
    ]
    
    currentmap = $mapname
    
    if (strcmp $gamemode "coopedit") [
        console script "Warning: running /newmap command as an unprivileged player will get you kicked and banned."
        event_handler $onmapchanged [ //cleanup on next map change
            if (! (strcmp $gamemode coopedit)) [
                cancel_handler
                mastermode @mastermode
                if (!= $currentmaster -1) [setmaster $currentmaster 0]
            ]
        ]
        if $allow_mm_locked [
            mastermode $MM_LOCKED
        ]
    ]
    
    if (> $gamecount 1) check_scriptpipe
    
    if (match ctf $gamemode) ctfsecscountdown
]

event_handler $onnewmap [
    parameters cn size
    if (unprivileged $cn) [
        kick $cn
        veto
    ] [
        log (format "%1(%2) set new map of size %3" (player_name $cn) $cn $size)
    ]
]

event_handler $onnewmap [
    parameters cn size
    
    log (format "%1(%2) set new map of size %3" (player_name $cn) $cn $size)
]

event_handler $onsetmaster [
    parameters cn set password success
    
    if $success [
        if (strcmp (player_priv $cn) "none") [
            log (format "%1(%2) relinquished privileged status." (player_name $cn) $cn)
            currentmaster = -1
        ] [
            log (format "%1(%2) gained %3" (player_name $cn) $cn (player_priv $cn))
            currentmaster = $cn
        ]
    ] [
        if (&& (&& (= $currentmaster -1) $set) (strcmp $gamemode "coopedit")) [
            setmaster $cn 1
            privconsole $cn script "You will lose master privilege when the server leaves coopedit mode."
        ]
    ]
]
    
event_handler $onauth [
    parameters cn success authname
    if $success [
        log (format "%1(%2) passed authentication as '%3'." (player_name $cn) $cn $authname)
        currentmaster = $cn
    ] [log (format "%1(%2) failed authentication as '%3'." (player_name $cn) $cn $authname)]
]

event_handler $onspectator [
    parameters cn spec
    if $spec [action = "joined"] [action = "left"]
    log (format "%1(%2) %3 spectators" (player_name $cn) $cn $action)
]

event_handler $ondeath [
    parameters offender victim
    
    teamkill_update $offender $victim
]

event_handler $onshutdown [
    
    log [Server shutdown @(datetime (now))]
    
    if $irc_enabled [
        log "Terminating the IRC bot"
        log_status "Waiting 2 seconds for the IRC bot to update..."
        server_sleep 2000 //give the ircbot time to read and broadcast the latest log messages
        kill $irc_pid
    ]
]

if $record_player_stats [exec scripts/stats/stats.csl]
if $autoteambalance [exec scripts/teambalance.csl]

defaultgame = [
    mastermode $MM_OPEN //must be reset in case it was set to locked or private from a server script
    
    if $default_on_empty [
        changemap $default_mode $default_map
        gametime $default_gametime
    ]
]

if $irc_enabled [
    daemon bin/irc.pl [] logs/irc.log logs/irc.log
]

flood_protection SV_TEXT        (secs 1)
flood_protection SV_SETMASTER   (secs 10)
flood_protection SV_KICK        (secs 30)
flood_protection SV_MAPVOTE     (secs 5)
flood_protection SV_C2SINIT     (secs 1)


// ============== Start of script pipe functions ==============

scriptpipe_filename = serverexec

open_scriptpipe = [
    if (path? $scriptpipe_filename) [
        log_error [Cannot create script pipe: "@scriptpipe_filename" filename already exists. Attempting to delete the existing file...]
        system [rm -f @scriptpipe_filename]
    ]
    script_pipe "bin/mkfifochan" [@scriptpipe_filename 777] []
    script_pipe_parse_timeout (secs 5)
]

check_scriptpipe = [
    if (! (path? $scriptpipe_filename)) [
        log_error ["@scriptpipe_filename" file has gone missing! Re-creating file...]
        open_scriptpipe
    ]
]

// ============== End of script pipe functions ==============

open_scriptpipe
if $script_socket_server_enabled [script_socket_server $script_socket_server_port]

try load_geoip_data "share/GeoIP.dat" [log_error "Expect 'country' function to fail because the GeoIP database was not loaded."]

printsvinfo = [
    parameters filename
    local output [  Server Title: @servername
  Max Players: @maxclients
  Running auto-update: @(yesno $auto_update)
  Running IRC bot: @(yesno $irc_enabled)
  Running stats database: @(yesno $record_player_stats)]
    system [echo "@output" >> @filename]
]

printsvstatus = [
    parameters filename
    local output (format "%1/%2, %3 %4, %5 mins left" $playercount $maxclients $mapname $gamemode $timeleft)
    system [echo "@output" >> @filename]
]

who = [
    local list [CN LAG PING IP COUNTRY NAME TIME STATE"\\n"]
    foreach (players) [
        parameters cn
        cty = "unknown"
        try [cty = (countrycode (player_ip $cn))] []
        row = (shell_quote [@cn @(player_lag $cn) @(player_ping $cn) @(player_ip $cn) @cty @(player_name $cn) @(duration (player_contime $cn)) @(player_status $cn)])
        list = (concatword $list [@row "\\n"])
    ]
    result (system [echo -e @(value list) | column -t])
]

system [touch conf/bans]
loadbans conf/bans

// ============== Start of map rotation functions ==============

nextmap = [
    parameters maplist
    local listsize (listlen $maplist)
    map = (at $maplist (mod $gamecount $listsize))
    if (strcmp $map $currentmap) [map = (at $maplist (mod (+ $gamecount 1) $listsize))]
    result $map
]

gamesize = [
    local count (- $playercount (countspecs))
    if(< $count 5) [result small] [
        if (< $count 8) [result medium] [result large]
    ]
]

mapsetname = [
    concatword (if $custom_maprotation_balance [result (concatword (gamesize) _)] [result ""]) $gamemode _maps
]

setnextmap = [
    local mapset (mapsetname)
    if (symbol? $mapset) [mapname (nextmap (value $mapset))] [log_error (concatword $mapset " not found, letting clients decide next map.")]
]

// ============== End of map rotation functions ==============

teamkill_update = [
    parameters offender victim
    local suicide (= $offender $victim)
    
    if (&& $teamkill_limit_enabled (&& (! $suicide) (&& (teamgame) (strcmp (player_team $offender) (player_team $victim))))) [
        local count (player_var $offender teamkills (+ 1 (player_var $offender teamkills)))
        local times (player_var $offender teamkill_times (concat $uptime (player_var $offender teamkill_times)))
        
        if (!= (player_var $offender teamkill_warned) 1) [
            privconsole $offender script [@(player_name $offender) you just killed a team mate, you should be shooting red players.]
            privconsole $offender script [Teamkill restrictions per game: maxcount=@teamkill_limit maxrate=@teamkill_maxrate per min.]
            player_var $offender teamkill_warned 1
        ]
        
        if (> $count 1) [
            msg (gameplay (format "%1 has fragged team mates %2 times." (print_name $offender) $count (player_gun $offender)))
        ]

        if (> (player_var $offender teamkills) $teamkill_limit) [
            kick $offender
            sleep (mins 30) [removeban (player_ip $offender)]
            console script [@(player_name $offender) was kicked for exceeding teamkill limit.]
            log (format "%1 was kicked for team killing." (player_name $offender))
        ] [
            if (&& (> (listlen $times) $teamkill_maxrate) [< (add_intervals $times $teamkill_maxrate) 60000]) [
                kick $offender
                sleep (mins 30) [removeban (player_ip $offender)]
                console script [@(player_name $offender) was kicked for exceeding maximum teamkill rate.]
                log (format "%1 was kicked for team killing." (player_name $offender))
            ]
        ]
    ]
]

ctfsecscountdown = [
    event_handler $ontimeupdate [
        parameters minsleft
        if (= $minsleft 1) [
            interval (secs 1) [
                local countdown (secsleft)
                if (= $countdown 30) [msg (gameplay "time remaining: 30 seconds")]
                if (= $countdown 15) [msg (gameplay "time remaining: 15 seconds")]
                if (= $countdown 10) [msg (gameplay "time remaining: 10 seconds")]
                if (= $countdown 0) stop
            ]
            cancel_handler
        ]
    ]
]

if (= $UID 0) [
    log_error "Running the server as root user is a serious security risk!"
    shutdown
]

if (= $check_pings 1) [
        interval (secs $check_pings_rate) [
                foreach (players) [if (> (player_ping $arg1) $maxping ) [
                playercn = $arg1
                player_var $playercn warnings (+ (player_var $playercn warnings) 1)
                if (> (player_var $playercn warnings) 3) [
                        msg (format "%1 %2" (blue (player_name $playercn)) (red [has a too high ping and will be kicked!]) )
                        log (format "%1 %2 %3" [PING:] (player_name $playercn) [get kicked!] )
                        kick $playercn
                        ]

                if (= (player_var $playercn warnings) 1) [
                privmsg $playercn (format "%1 " (red [WARNING: You will get kicked after 4 warnings!]) ) ]

                msg (format "%1 %2 %3 %4 %5 %6" (red [WARNING:]) (red [(]) (blue (player_var $playercn warnings)) (red[)]) (blue (player_name $playercn)) (red [Your PING is too high!]) )
                ]]
        ]
]

if (= $check_unnameds 1) [
        interval (secs 10) [
                foreach (players) [if (strcmp (player_name $arg1) "unnamed") [
                        playeruncn = $arg1

                        if (<= (player_var $playeruncn warningsun) 3) [
                        player_var $playeruncn warningsun (+ (player_var $playeruncn warningsun) 1)
                        privmsg $playeruncn (format "%1 %2 %3 %4 %5" (red [WARNING:]) [(] (blue (player_var $playeruncn warningsun)) [)] (red [You will get speced after 4 warnings, if you dont change your name now!]) )
                        ]

                        if (> (player_var $playeruncn warningsun) 3) [
                        spec $playeruncn
                        privmsg $playeruncn (format "%1 %2 %3" (red [WARNING:]) (blue [Unnameds]) (red [are not allowed to play here!
         Please type /name yourname and reconnect to the server!]) )
                                ]
                        ]
                ]
        ]
]

if (is_startup) [
    log [Server started @(datetime (now))]
    
    currentmaster = -1
    allow_talk = 1
    disallow_talk_reason = ""

    defaultgame
] [
    log [Reloaded server startup scripts @(datetime (now))]
]

log_status "*** Running Hopmod 3.0 for Sauerbraten Development Version ***"
