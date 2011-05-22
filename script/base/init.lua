package.path = package.path .. ";script/package/?.lua;"
package.cpath = package.cpath .. ";lib/lib?.so"

dofile("script/base/core_function_overloads.lua")
dofile("script/base/event.lua")
dofile("script/base/server.lua")
dofile("script/base/cubescript.lua")
dofile("script/base/serverexec.lua")
-- The exec function is available after cubescript.lua has executed

add_exec_search_path("script")
add_exec_search_path("conf")

exec("base/resetvars.cs")
exec("base/resetvars.lua")
exec("base/module.lua")
exec("base/utils.lua")
exec("base/logging.lua")
exec("base/restart.lua")
exec("base/player/utils.lua")
exec("base/player/vars.lua")
exec("base/player/object.lua")
exec("base/player/iterators.lua")
exec("base/player/query.lua")
exec("base/player/private_vars.lua")
exec("base/player/command.lua")
exec("base/team/utils.lua")
exec("base/setmaster.lua")
exec("base/kickban.lua")
exec("base/server_message.lua")
exec("base/static_items.lua")

server.module("base/maprotation/init.lua")
server.module("base/normalize")
server.module("base/modified_map")
server.module("base/banner")
server.module("base/mute")
server.module("base/auth/init")
server.module("base/mapvote")
server.module("base/register_server")
server.module("base/web/init")
server.module("base/teamkills")
server.module("base/global_bans")

-- Load module configuration variables
local function load_module_vars(path)

    local filesystem = require "filesystem"
    
    for filetype, filename in filesystem.dir(path) do
        
        local fullfilename = path .. "/" .. filename
        
        if (filetype == filesystem.DIRECTORY) and (filename ~= "." and filename ~= "..") then
            load_module_vars(fullfilename)
        elseif (filetype == filesystem.FILE or filetype == filesystem.UNKNOWN) and filename:match(".vars$") then
            exec(fullfilename)
        end
    end
end

load_module_vars("./script/module/declare")

server.event_handler("started", function()
    
    server.reload_maprotation()
    
    require("geoip").load_geoip_database(server.geoip_db_file)

    log_unknown_player_commands()
    
    server.log_status("-> Successfully loaded Hopmod")
end)

server.event_handler("shutdown", function() 
    server.log_status("Server shutting down.")
end)

exec_if_found("conf/server.conf")
exec("base/saveconf.lua")

