
interval $ping_limiter_tick [

    if (> $playercount 2) [
        
        my cn 0
        my warnings 0
    
        foreach (players) [
            
            set cn $arg1
            
            if (> (player_ping $cn) $ping_limit) [
                
                set warnings (+ 1 (player_var $cn ping_warnings))
                player_var $cn ping_warnings $warnings
                
                if (> $warnings $ping_limit_warnings) [
                    log [Net stats for @(player_name $cn): ping @(player_ping $cn) lag @(player_lag $cn)]
                    kick $cn 0 "server" "high ping"
                    player_var $cn ping_warnings 0
                ] [
                    player_msg $cn [@(green (player_name $cn)), your ping is too high for this server - it needs to be below @ping_limit ms.]
                ]
            
            ]
        ]
        
    ]
    
    if (= $enable_ping_limiter 0) [return -1] //stop interval
]
