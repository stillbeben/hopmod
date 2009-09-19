# Do not delete this file. The game server expects this script "script/init.cs"
# to exist. This is the first script to be executed by the game server.

exec "./script/resetvars.cs"
exec "./script/module.lua"
exec "./script/serverlib.lua"
exec "./script/servermod.lua"
exec "./script/servermod.cs"

execIfFound "./conf/server.conf"
