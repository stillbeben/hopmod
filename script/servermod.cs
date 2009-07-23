
exec "script/playercmd.cs"
exec "script/irc.csl"

global execCubeScriptFile &exec-cubescript

event_handler "started" (func [] [
    if $enable_ping_limiter [exec script/pinglimiter.cs]
    if $enable_cheater_command [exec script/cheatercmd.cs]
])
