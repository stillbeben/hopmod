
interval (mins 2) [
    msg (blue [If you suspect a cheater type "#cheater <cn>" to notify an admin.])
    msg (blue [Put a client number where <cn> is, to see players' CNs type "/showclientnum 1"])
]

global playercmd_cheater (func [cn cheat] [

    if (&& (!= (player_sessionid $cheat) -1) 1) [
        
        player_pvar $cn cheat_report (+ 1 (player_pvar $cn cheat_report))
        
        if (> (player_pvar $cn cheat_report) 4) [
            player_msg $cn "Don't spam with the #cheater command or you will be ignored."
        ]
        
        if (< (player_pvar $cn cheat_report) 8) [
            
            if (> (player_connection_time $cheat) 3) [
                
                player_msg $cn "Thank you for your report, hopefully an admin will check this out very soon."
                log (format "CHEATER: %1 %2" (player_name $cn) (player_name $cheat))
            
            ] [player_msg $cn (player_connection_time $cheat); player_msg $cn "This player has just connected, are you sure? Type it again."]
        ]
        
    ]
])
