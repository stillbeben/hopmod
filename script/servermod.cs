
mins = [* $arg1 60000]
secs = [* $arg1 1000]

concol = [format "s%1%2r" $arg1 $arg2]
green = [concol 0 $arg1]
info = [concol 1 $arg1]
err = [concol 3 $arg1]
grey = [concol 4 $arg1]
magenta = [concol 5 $arg1]
orange = [concol 6 $arg1]
gameplay = [concol 2 $arg1]
red = [concol 3 $arg1]
blue = [concol 1 $arg1]
yellow = [concol 2 $arg1]

exec "script/playercmd.cs"
exec "script/irc.csl"

event_handler "started" (func [] [
    if $use_ping_limiter [exec script/pinglimiter.cs]
])
