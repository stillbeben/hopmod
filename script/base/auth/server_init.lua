package.path = package.path .. ";script/package/?.lua"
package.cpath = package.cpath .. ";lib/lib?.so"
dofile("./script/base/utils.lua")

function server.load_sqlite3_database(filename)

    require "sqlite3"
    require "sqlite3utils"
    
    local db, perr = sqlite3.open(filename)
    if not db then error(perr) end
    sqlite3utils.createMissingTables("./script/base/auth/db_schema.sql", db)
    
    local select_users_sql = "SELECT users.name as name, users.pubkey as pubkey, domains.name as domain FROM users INNER JOIN domains ON users.domain_id = domains.id"

	if (server.authnames_case_insensitive or 0) == 1 then
		for row in db:rows(select_users_sql) do
			server.adduser(row.name, row.domain, row.pubkey)
			server.adduser(string.lower(row.name), row.domain, "")
		end
	else
		for row in db:rows(select_users_sql) do
			server.adduser(row.name, row.domain, row.pubkey)
		end
	end
    
    db:close()
end

server.execute_cubescript_file("./conf/authserver.conf")
