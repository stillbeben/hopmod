
function createMissingTables(schemafilename, db)
        
    schemafile,err = io.open(schemafilename)
    if not schemafile then return nil,err end
    
    local schema = sqlite3.open_memory()
    schema:exec("BEGIN TRANSACTION")
    schema:exec(schemafile:read("*a"))
    schema:exec("COMMIT TRANSACTION")
    
    db:exec("BEGIN TRANSACTION")
    
    for row in schema:rows("SELECT * FROM sqlite_master") do
        --FIXME find a way to reset prepared statement
        local search_for_table,perr = db:prepare("SELECT count(*) as count FROM sqlite_master WHERE name = ?")
        search_for_table:bind(row.name)
        
        local table_count,perr = search_for_table:first_row()
        if table_count.count == 0 then
            
            db:exec(row.sql)
            
        else -- table found, check columns
            
            local existing_cols = {}
            
            for table_column in db:rows(string.format("PRAGMA table_info(%s)", row.name)) do
                existing_cols[table_column.name] = table_column
            end
            
            for table_column in schema:rows(string.format("PRAGMA table_info(%s)", row.name)) do
                if not existing_cols[table_column.name] then
                    
                    local column_def = table_column.name .. " " .. table_column.type
                    
                    if table_column.pk == 1 then
                        column_def = column_def .. " PRIMARY KEY"
                    else
                        if table_column.notnull == 1 then
                            column_def = column_def .. " NOT NULL"
                        elseif table_column.dflt_value then
                            column_def = column_def .. " DEFAULT " .. table_column.dflt_value
                        end
                    end
                    
                    db:exec(string.format("ALTER TABLE %s ADD COLUMN %s", row.name, column_def))
                    
                end
            end
        end
    end
    
    db:exec("COMMIT TRANSACTION")
end
