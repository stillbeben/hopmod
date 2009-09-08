
local using_sqlite = (server.stats_use_sqlite == 1)
local using_json = (server.stats_use_json == 1)

local backends = {}

if using_sqlite then
    
    backends.sqlite3 = loadfile("./script/db/stats/sqlite3.lua")()
    
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
    
    backends.json = loadfile("./script/db/stats/json.lua")()
    
end

loadfile("./script/db/stats/core.lua")().initialize(backends,{
        using_auth = server.stats_use_auth,
        auth_domain_name = server.stats_auth_domain
    })

if server.enable_stats_command == 1 then
    loadfile("./script/db/stats/playercmd.lua")().initialize(backends)
end
