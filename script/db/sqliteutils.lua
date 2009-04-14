
function createMissingTables(schemafilename, db)
    
    schemafile,err = io.open(schemafilename)
    if not schemafile then return nil,err end
    
    local schema = sqlite3.open_memory()
    schema:exec("BEGIN TRANSACTION")
    schema:exec(schemafile:read("*a"))
    schema:exec("COMMIT TRANSACTION")
    
    db:exec("BEGIN TRANSACTION")
    
    for row in schema:rows("SELECT * FROM sqlite_master") do
        local search = db:prepare("SELECT count(*) as count FROM sqlite_master WHERE name = ?")
        search:bind(row.name)
        if search:first_row().count == 0 then
            db:exec(row.sql)
        end
    end
    
    db:exec("COMMIT TRANSACTION")
end
