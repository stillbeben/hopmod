
package.path = package.path .. ";script/package/?.lua"
package.cpath = package.cpath .. ";lib/lib?.so"

load_once("base/restart")
load_once("base/player/utils")
load_once("base/player/vars")
load_once("base/player/object_wrapper")
load_once("base/player/query")
load_once("base/player/global_vars")
load_once("base/player/private_vars")
load_once("base/player/command")
load_once("base/logging")
load_once("base/maprotation") -- the reason this can be loaded now instead as a scheduled module is because of reload_maprotation is called from started event

server.module("base/banner")
server.module("base/mute")
server.module("base/force_spec")
server.module("base/auth/init")
server.module("base/mapvote")
server.module("base/register_server")
server.module("base/web/init")

local function update_gamemodeinfo()
    gamemodeinfo = server.gengamemodeinfo()
end

server.event_handler("mapchange", function(map, mode)
    update_gamemodeinfo()
end)

update_gamemodeinfo()

-- Load module configuration variables
local function load_module_vars(path)

    local filesystem = require "filesystem"
    
    for filetype, filename in filesystem.dir(path) do
        
        local fullfilename = path .. "/" .. filename
        
        if filetype == filesystem.DIRECTORY and (filename ~="." and filename ~= "..") then
            load_module_vars(fullfilename)
        elseif filetype == filesystem.FILE and filename:match(".vars$") then
            server.execute_cubescript_file(fullfilename)
        end
    end
end

load_module_vars("./script/module/declare")

server.event_handler("started", function()
    
    if server.file_exists(server.banlist_file) then
        server.execCubeScriptFile(server.banlist_file)
    end
    
    if server.file_exists(server.lua_ban_file) then
        load_once(server.lua_ban_file)
    end
        
    server.reload_maprotation()
    
    require("geoip").load_database(server.geoip_db_file)
    
    load_once("command/_bindings")
    log_unknown_player_commands()
    
    server.log_status("-> Successfully loaded Hopmod")
    
end)

server.event_handler("shutdown", function() 
    server.log_status("Server shutting down.")
end)
