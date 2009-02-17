
print_current_stats = [
    parameters cn
    local frags (player_var $cn frags)
    local deaths (player_var $cn deaths)
    local suicides (player_var $cn suicides)
    local damage (player_var $cn damage)
    local acc (player_accuracy $cn)
    
    local suicides_msg ""
    if (> $suicides 0) [suicides_msg = [(@suicides suicides) ]]
    
    privmsg $cn (format "Frags: %1 Deaths: %2 %3Accuracy: %4" (green $frags) (red $deaths) (red $suicides_msg) (yellow $acc))
    
    if (ctfp) [
        local scored (player_var $cn ctf_scored)
        local pickups (player_var $cn ctf_pickups)
        local drops (player_var $cn ctf_drops)
        local returns (player_var $cn ctf_returns)
        local defended (player_var $cn ctf_defended)
        
        privmsg $cn (format "Scored: %1 Pickups: %2 Drops: %3 Returns: %4 Defended: %5" (green $scored) (yellow $pickups) (red $drops) (green $returns) (green $defended))
    ]
    
    privmsg $cn [
To see your total stats for this server type s0#stats totalr]
]

print_total_stats = [
    parameters cn
    local name (player_name $cn)
    
    local frags 0
    local deaths 0
    local suicides 0
    local maxfrags 0
    local hits 0
    local misses 0
    local accuracy 0
    local gamecount 0
    local kpd 0
    
    statsdb eval [select coalesce(sum(frags),0) as total_frags,
                         coalesce(sum(deaths),0) as total_deaths,
                         coalesce(max(frags),0) as max_frags,
                         round(sum(suicides)/(sum(deaths)+0.0)*100) as suicideper,
                         sum(hits) as total_hits,
                         sum(misses) as total_misses,
                         round(sum(hits)/(sum(hits)+max(sum(misses),1)+0.0)*100) as accuracy,
                         round(sum(frags)/(sum(deaths)+0.0),2) as kpd,
                         coalesce(count(*),0) as gamecount
                    from players where name=$name] [
        frags = (round (column total_frags))
        deaths = (round (column total_deaths))
        suicides = (concatword (round (column suicideper)) %)
        maxfrags = (round (column max_frags))
        hits = (column total_hits)
        misses = (column total_misses)
        accuracy = (concatword (round (column accuracy)) %)
        gamecount = (round (column gamecount))
        kpd = (column kpd)
    ]
    
    if (> $misses 0) [
        
        local suicides_msg [(@suicides suicides) ]
        //if (> $suicides 0) [suicides_msg = ]
        
        //sqlite's max aggregate function returns empty text for some players, I don't know why...
        if (strcmp $maxfrags "") [maxfrags = 0]
        
        privmsg $cn (format "Games: %1 Frags: %2 Deaths: %3 %4Accuracy: %5 KPD: %6" (yellow $gamecount) (green $frags) (red $deaths) (red $suicides_msg) (yellow $accuracy) (yellow $kpd))
        privmsg $cn (format "Max Frags: %1" (green $maxfrags))
        
    ] [privmsg $cn (err "No history found.")]
]

playercmd_stats = [
    local argc $numargs //FIXME - numargs not visible inside if
    if $record_player_stats [
        if (= $argc 0) [print_current_stats $cn] [
            local command $arg1
            if (strcmp $command "total") [print_total_stats $cn] [print_current_stats $cn]
        ]
    ] [privmsg $cn (err "Player statistics logging is disabled on this server.")]
]
