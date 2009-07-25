do = [arg1]

exec "script/playercmd.cs"
exec "script/irc.csl"

global execCubeScriptFile &exec-cubescript

event_handler "started" (func [] [
    if $enable_ping_limiter [exec script/pinglimiter.cs]
    if $enable_cheater_command [exec script/cheatercmd.cs]
    
    if $enable_eval_command [
        global playercmd_eval (func [cn code] [
            if (strcmp (player_priv $cn) "admin") [do [@code]] [player_msg $cn (err "Permission Denied")]
        ])
    ]
])
