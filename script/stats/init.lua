
local using_sqlite = (server.stats_use_sqlite == 1)
local using_json = (server.stats_use_json == 1)
local using_mysql = (server.stats_use_mysql == 1)

local backends = {}

if using_sqlite then
    
    backends.sqlite3 = loadfile("./script/stats/sqlite3.lua")()
    
    local is_open, err = backends.sqlite3.open({
        filename = server.stats_db_filename, 
        schemafile = "./script/db/stats/schema.sql",
        exclusive_locking = server.stats_sqlite_exclusive_locking,
        synchronous = server.stats_sqlite_synchronous})
        
    if not is_open then
        error(err) -- log but don't throw
    end
    
    backends.query = backends.sqlite3
end

if using_json then
    backends.json = loadfile("./script/stats/json.lua")()    
end

if using_mysql then
    
    backends.mysql = dofile("./script/stats/mysql.lua")
    
    backends.mysql.open({
        hostname = server.stats_mysql_hostname,
        port = server.stats_mysql_port,
        username = server.stats_mysql_username,
        password = server.stats_mysql_password,
        database = server.stats_mysql_database,
        schema = "./script/stats/mysql_schema.sql",
        triggers = "./script/stats/mysql_triggers.sql",
        install = server.stats_mysql_install == 1
    })
    
end

loadfile("./script/stats/core.lua")().initialize(backends,{
        using_auth = server.stats_use_auth,
        auth_domain_name = server.stats_auth_domain
    })

-- Load and register the #stats player command
local stats_command = loadfile("./script/stats/playercmd.lua")()
stats_command.initialize(backends)
player_command_function("stats", stats_command.command_function)

-- 
server.find_names_by_ip = backends.query.find_names_by_ip

if not server.find_names_by_ip then 
    server.find_names_by_ip = function() return nil end
end
