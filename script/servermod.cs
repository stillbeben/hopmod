do = [arg1]

exec "script/irc.csl"

global execCubeScriptFile &exec-cubescript

global execIfFound (func [filename] [
    if (fileExists $filename) [exec $filename]
])
