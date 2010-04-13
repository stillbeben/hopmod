--[[
    See script/stats/core.lua for information
]]

local using_sqlite = (server.stats_use_sqlite == 1)
local using_json = (server.stats_use_json == 1)
local using_mysql = (server.stats_use_mysql == 1)

local backends = {}

function server.stats_wrapper(name)
    return (backends.query.player_totals(name) or nil)
end

if using_sqlite then
    
    backends.sqlite3 = loadfile("./script/module/stats/sqlite3.lua")()
    
    local is_open, err = catch_error(backends.sqlite3.open,{
        filename = server.stats_db_filename, 
        schemafile = "./script/module/stats/schema.sql",
        exclusive_locking = server.stats_sqlite_exclusive_locking,
        synchronous = server.stats_sqlite_synchronous})
    
    if not is_open then
        error(err) -- log but don't throw
    end
    
    --catch_error(server.stats_sqlite_reinstall_triggers) -- Causes the database connection to break (reason unknown)
    
    backends.query = backends.sqlite3
end



if using_json then
    backends.json = catch_error(loadfile("./script/module/stats/json.lua"))    
end

if using_mysql then
    
    backends.mysql = dofile("./script/module/stats/mysql.lua")
    
    catch_error(backends.mysql.open,{
        hostname = server.stats_mysql_hostname,
        port = server.stats_mysql_port,
        username = server.stats_mysql_username,
        password = server.stats_mysql_password,
        database = server.stats_mysql_database,
        schema = "./script/module/stats/mysql_schema.sql",
        triggers = "./script/module/stats/mysql_triggers.sql",
        install = server.stats_mysql_install == 1,
        servername = server.stats_servername
    })
    
end

dofile("./script/module/stats/core.lua").initialize(backends,{
        using_auth = server.stats_use_auth,
        auth_domain = server.stats_auth_domain,
        gamemodes = list_to_set(server.parse_list(server.stats_enabled_gamemodes))
    })

-- Load and register the #stats player command
local stats_command = loadfile("./script/module/stats/playercmd.lua")()
stats_command.initialize(backends)
player_command_function("stats", stats_command.command_function)

-- 

if backends.query then
    server.find_names_by_ip = backends.query.find_names_by_ip
end

if not server.find_names_by_ip then 
    server.find_names_by_ip = function() return nil end
end
