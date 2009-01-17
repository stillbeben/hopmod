
//A player command that allows admin players to execute cubescript code on the server in-game.
//Syntax is "#eval [<code>]", with the square brackets included and <code> being your code.
playercmd_eval = [
    if (strcmp (player_priv $cn) "admin") [do [@arg1]] [privmsg $cn (err "Permission Denied")]
]

playercmd_script = [
    local filename (format "scripts/%1" $arg1)
    playercmd_eval [exec "@filename"]
]

playercmd_specall = [
    if (strcmp (player_priv $cn) "admin") [
        foreach (players) [spec $arg1]
    ] [privmsg $cn (err "Permission Denied")]
]

playercmd_unspecall = [
    if (strcmp (player_priv $cn) "admin") [
        foreach (players) [unspec $arg1]
    ] [privmsg $cn (err "Permission Denied")]
]

playercmd_names = [
    parameters target
    if (strcmp (player_priv $cn) "none") [throw runtime.playercmd.names.permission_denied]
    privmsg $cn (showaliases $target)
]

playercmd_who = [
    if (strcmp (player_priv $cn) "none") [throw runtime.playercmd.names.permission_denied]
    privmsg $cn (who)
]

playercmd_invadmin = [
    parameters pass
    if (adminpass $pass) [setpriv $cn admin] [privmsg $cn (err "Command Failed")]
]

playercmd_group = [
    if (>= (listlen $arguments) 2) [
	reference tag arg1
        reference t_team arg2
        foreach (players) [
                if ((match $tag (player_name $arg1))) [ msg (err (format "Moving %2 to %1 for team grouping" $t_team (player_name $arg1)) ) ; setteam $arg1 $t_team ]
        ]
    ] [privmsg $cn (err "Missing argument. Allows team grouping of players by tag. Syntax #group <tag> <team>.")]
]

playercmd_setmotd = [
    if (>= (listlen $arguments) 1) [
	motd = $arg1
	privmsg $cn (format "MOTD Changed to %1" (magenta $motd) )
    ]
]

playercmd_slap = [
    parameters target
    if (strcmp (player_priv $cn) "admin") [
        msg (grey [*@(concol 7 (player_name $target)) got slapped by @(orange (player_name $cn))*])
    ] [privmsg $cn (err "Permission Denied")]
]
    
playercmd_1on1 = [
        parameters target
        if (strcmp (player_priv $cn) "admin") [

        player1 = $arg1
        player2 = $arg2
        game_mo = $arg3
        gamemap = $arg4

        if (strcmp $game_mo "insta") [game_mo = instagib]
        if (strcmp $game_mo "instateam") [game_mo = [instagib team]
                setteam $player1 linux
                setteam $player2 windows
        ]
        if (strcmp $game_mo "ffa") [game_mo = ffa/default]
        if (strcmp $game_mo "eff") [game_mo = efficiency]
        if (strcmp $game_mo "ictf") [game_mo = [insta ctf] ]
        if (strcmp $game_mo "icapture" ) [game_mo = [insta capture] ]


        player1con = (player_conid $player1)
        player2con = (player_conid $player2)
        log (format "%1 %2 %3 %4 %5 %6" [MATCH:] (player_name $player2) [versus] (player_name $player1) [on] $gamemap )
        mastermode 2
        foreach (players) [spec $arg1]
        msg (format "%1 %2 %3 %4 %5 %6 %7" (orange (player_name $player1)) (grey [versus]) (orange (player_name $player2)) (grey [on]) (grey $game_mo) (orange $gamemap) (grey [in 10 secs]) )
        sleep (secs 5) [ msg (format "%1" (red [Game will start in 5 secs...]) )
        sleep (secs 1) [ msg (format "%1" (red [Game will start in 4 secs...]) )
        sleep (secs 1) [ msg (format "%1" (red [Game will start in 3 secs...]) )
        sleep (secs 1) [ msg (format "%1" (red [Game will start in 2 secs...]) )
        sleep (secs 1) [ msg (format "%1" (red [Game will start in 1 secs...]) )
        msg (format "%1 %2" (red [Map will change now to:]) (orange $gamemap) )
        sleep (secs 2) [
        changemap $game_mo $gamemap
        if (strcmp $game_mo "[instagib team]") [
                setteam $player1 linux
                setteam $player2 windows
        ]
        unspec $player1
        unspec $player2
        msg (format "%1" (blue [Start the fight now!!!]) )

        event_handler $onintermission [
        msg (format "%1" (orange [Good Game]) )

        if (&& (= $player1con (player_conid $player1)) (= $player2con (player_conid $player2)) ) [

        if (= (player_frags $player1) (player_frags $player2)) [
        msg (format "%1 %2 %3 %4" (blue [Oh NO winner:]) (orange(player_frags $player1)) (blue [-]) (orange(player_frags $player2)) )
        log (format "%1 %2 %3 %4 %5" [MATCH:] [Game has end! A draw!] (player_name $player2) [versus] (player_name $player1) )
        ] [
                if (> (player_frags $player1) (player_frags $player2)) [
                winner = $player1
                loser = $player2
                ] [
                winner = $player2
                loser = $player1
                ]
                log (format "%1 %2 %3 %4 %5 %6 %7 %8" [MATCH: Game has end!] (player_name $winner) [wins, with] (player_frags $winner) [-] (player_frags $loser) [Poor] (player_name $loser) )
                msg (format "%1 %2 %3 %4 %5" (blue [The Winner is:]) (orange (player_name $winner)) (blue [with]) (orange (player_frags $winner)) (orange [FRAGS!]) )
                msg (format "%1 %2 %3 %4 %5" (blue [Sorry]) (orange (player_name $loser)) (blue [you have only]) (orange (player_frags $loser)) (orange [FRAGS!]) )
        ]
        ] [ msg (format "%1" (red [Can't display the score. Maybe one of the players has been disconnected during the game!]) )
            msg (format "%1" (red [But you can take a look at the scoreboard!]) ) ]

        cancel_handler]

        ]]]]]]

        ] [privmsg $cn (err "You need admin to do that!")]
]

playercmd_pause = [
    if (strcmp (player_priv $cn) "admin") [
        pausegame
    ] [privmsg $cn (err "Permission Denied")]
]

playercmd_resume = [
    if (strcmp (player_priv $cn) "admin") [
        resumegame
    ] [privmsg $cn (err "Permission Denied")]
]

playercmd_msg = [
        yourcn = $cn
        playercnm = $arg1
        playermsg = $arg2
        privmsg $playercnm (format "%1 %2 %3" (blue (player_name $yourcn)) [:] (red $playermsg) )
        privmsg $yourcn (format "%1 %2 %3" (blue $playermsg) (red [was send to:]) (red (player_name $playercnm)) )
	
]

