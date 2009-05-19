
global playercmd_1on1 (func [cn arg1 arg2 arg3 arg4] [

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

    player1con = (player_sessionid $player1)
    player2con = (player_sessionid $player2)
    
    log (format "%1 %2 %3 %4 %5 %6" [MATCH:] (player_name $player2) versus (player_name $player1) on $gamemap )
    
    mastermode 2
    specall
    
    unspec $player1
    unspec $player2
    
    msg (format "%1 %2 %3 %4 %5 %6 %7" (green (player_name $player1)) versus (green (player_name $player2)) on (green $game_mo) (green $gamemap) [in 10 secs] )
    
    sleep (secs 5) [ msg (format "%1" [Game will start in 5 seconds...])
    sleep (secs 1) [ msg (format "%1" [Game will start in 4 seconds...])
    sleep (secs 1) [ msg (format "%1" [Game will start in 3 seconds...])
    sleep (secs 1) [ msg (format "%1" [Game will start in 2 seconds...])
    sleep (secs 1) [ msg (format "%1" [Game will start in 1 second...])
    msg (format "%1 %2" [Map will change now to:] (green $gamemap) )
    sleep (secs 2) [
        
        changemap $gamemap $game_mo 
        
        if (strcmp $game_mo "[instagib team]") [
            setteam $player1 lin
            setteam $player2 win
        ]

        msg (format "%1" [Start the fight now!!!] )
        
        1on1_intermission = (event_handler intermission (func [] [
        
            if (&& (= $player1con (player_sessionid $player1)) (= $player2con (player_sessionid $player2)) ) [
                msg "Good Game!"
                
                my p1_frags (player_frags $player1)
                my p2_frags (player_frags $player2)
                
                if (= $p1_frags $p2_frags) [
                    msg (format "%1 %2 %3 %4" [Oh NO winner:] (green (player_frags $player1)) [-] (green (player_frags $player2)) )
                    log (format "%1 %2 %3 %4 %5" [MATCH:] [Game has ended in a draw!] (player_name $player1) versus (player_name $player2) )
                ] [
                    
                    my winner
                    my loser
                    
                    if (> $p1_frags $p2_frags) [
                        set winner $player1
                        set loser $player2
                    ] [
                        set winner $player2
                        set loser $player1
                    ]
                    
                    log (format "%1 %2 %3 %4 %5 %6 %7 %8" [MATCH: Game has end!] (player_name $winner) [wins, with] (player_frags $winner) [-] (player_frags $loser) [Poor] (player_name $loser) )
                    
                    msg (format "%1 %2 %3 %4 %5" [The Winner is:] (green (player_name $winner)) with (green (player_frags $winner)) [FRAGS!])
                    msg (format "%1 %2 %3 %4 %5" [Sorry] (red (player_name $loser)) [you have only] (red (player_frags $loser)) [FRAGS!] )
                ]
                
            ] [
                msg (red "Can't display the score. Maybe one of the players has been disconnected during the game!")
                msg (red "But you can take a look at the scoreboard!")
            ]
            
            cancel_handler $1on1_intermission
        ]))
        
    ]]]]]]

    ] [player_msg $cn (red "You need admin to do that!")]
])
