
print_current_stats = [
    parameters cn
    local frags (player_var $cn frags)
    local deaths (player_var $cn deaths)
    local suicides (player_var $cn suicides)
    local damage (player_var $cn damage)
    local acc (player_accuracy $cn)
    local suicides_msg ""
    if (> $suicides 0) [suicides_msg = [(@suicides suicides) ]]
    privmsg $cn (info (format "Frags: %1 Deaths: %2 %3Accuracy: %4" (green $frags) (red $deaths) (red $suicides_msg) (yellow $acc)))
    if (ctfp) [
        local scored (player_var $cn ctf_scored)
        local pickups (player_var $cn ctf_pickups)
        local drops (player_var $cn ctf_drops)
        local returns (player_var $cn ctf_returns)
        local defended (player_var $cn ctf_defended)
        privmsg $cn (info (format "Scored: %1 Pickups: %2 Drops: %3 Returns: %4 Defended: %5" (green $scored) (yellow $pickups) (red $drops) (green $returns) (green $defended)))
    ]
]

print_total_stats = [
    parameters cn
    local name (player_name $cn)
    
    local frags 0
    local deaths 0
    local suicides 0
    local max_frags 0
    local accuracy 0
    local gamecount 0
    local kpd 0
    statsdb eval [select sum(frags) as total_frags,
                         sum(deaths) as total_deaths,
                         sum(suicides) as total_suicides,
                         max(frags) as max_frags,
                         sum(hits) as total_hits,
                         sum(misses) as total_misses,
                         count(*) as gamecount
        from players where name=$name] [
        frags = (column total_frags)
        deaths = (column total_deaths)
        suicides = (column total_suicides)
        max_frags = (column max_frags)
        local hits (column total_hits)
        local misses (column total_misses)
        accuracy = (concatword (round (fmul (fdiv $hits (max (+ $hits $misses) 1)) 100)) %)
        gamecount = (column gamecount)
        kpd = (fdiv $frags $deaths)
    ]
    local suicides_msg ""
    if (> $suicides 0) [suicides_msg = [(@suicides suicides) ]]
    privmsg $cn (info (format "Games: %1 Frags: %2 Deaths: %3 %4 Accuracy: %5 KPD: %6" (yellow $gamecount) (green $frags) (red $deaths) (red $suicides_msg) (yellow $accuracy) (yellow $kpd)))
    privmsg $cn (info (format "Max Frags: %1" (green $max_frags)))
]

stats_cmd_total_hook = []
stats_cmd_game_hook = []

playercmd_stats = [
    local argc $numargs //FIXME - numargs not visible inside if
    if $record_player_stats [
        if (= $argc 0) [print_current_stats $cn; stats_cmd_game_hook $cn] [
            local command $arg1
            if (strcmp $command "total") [print_total_stats $cn; stats_cmd_total_hook $cn] []
        ]
    ] [privmsg $cn (err "Player statistics logging is disabled on this server.")]
]
