require "sqlite3"
dofile("./script/db/sqliteutils.lua")

local db = sqlite3.open(server.auth_db_filename)
createMissingTables("./script/db/auth_schema.sql", db)

local perr
local search_for_domain,perr = db:prepare("SELECT * FROM domains WHERE name = :name")
if not search_for_domain then error(perr) end

local search_for_user, perr = db:prepare("SELECT * FROM users WHERE domain_id = :domain_id and name = :name")
if not search_for_user then error(perr) end

local insert_user, perr = db:prepare("INSERT INTO users (domain_id, name, pubkey) VALUES (:domain_id,:name,:pubkey)")
if not insert_user then error(perr) end

local insert_domain, perr = db:prepare("INSERT INTO domains (name, local) VALUES (:name,:local)")
if not insert_domain then error(perr) end

local auth_request = {}

auth_domain_handlers = {}
auth = {}

server.event_handler("authreq", function(cn,name,domain)
    
    local req_id = server.player_authreq(cn)
    auth_request[req_id] = {}
    local req = auth_request[req_id]
    
    search_for_domain:bind{name = domain}
    local row = search_for_domain:first_row()

    req.name = name
    
    if row then
        req.domain = domain
        req.domain_id = row.domain_id
    end
    
    if not row or row["local"] == 0 then
    
        req.delegated = true
        server.delegateauth(cn)

    else
   
        req.delegated = false
        
        search_for_user:bind{domain_id = row.id, name = name}
        local row = search_for_user:first_row()
        
        if not row then
            server.signal_auth_failure(cn)
        else
        
            local ans,chal = server.genchallenge(row.pubkey)
            req.answer = ans
            server.sendauthchallenge(cn, chal)
            
        end
    end
    
end)

server.event_handler("authrep", function(cn, answer)

    local req_id = server.player_authreq(cn)
    local req = auth_request[req_id]
    
    if not req then
        error("auth reply to non-existent request")
    end
    
    if req.delegated then
        server.relayauthanswer(cn, answer)
        return
    end

    if server.checkchallenge(answer, req.answer) then
        server.signal_auth_success(cn)
    else
        server.signal_auth_failure(cn)
    end

end)

server.event_handler("auth", function(cn, name, domain, success)
    
    local req_id = server.player_authreq(cn)
    local req = auth_request[req_id]
    
    if not req.delegated then
        server.freechalanswer(req.answer)
    end
    
    if tonumber(success) == 1 then
        if req.domain then
            
            local handler = auth_domain_handlers[req.domain]
            if handler then
                handler(cn, name)
            end
        else
            if server.player_priv_code(cn) == 0 then
                server.msg(string.format("%s claimed master as '%s'",server.player_name(cn),magenta(name)))
                server.setmaster(cn)
            end
        end
    end
    
    auth_request[req_id] = nil
    
end)

function auth.get_domain_id(name)
    search_for_domain:bind{name = name}
    local row = search_for_domain:first_row()
    if not row then return end
    return row.id
end

function auth.found_name(name,domain_id)
    search_for_user:bind{domain_id = domain_id, name = name}
    local row = search_for_user:first_row()
    if row then return true else return false end
end

function auth.add_user(name, pubkey, domain)
    local domain_id = auth.get_domain_id(domain)
    if not domain_id then error("domain not found") end
    if auth.found_name(name, domain_id) then error("user name already in use") end
    insert_user:bind{domain_id = domain_id, name = name, pubkey = pubkey}
    insert_user:exec()
end

function auth.add_domain(name, islocal)
    local domain_id = auth.get_domain_id(name)
    if domain_id then return domain_id end
    if not islocal then islocal = true end
    insert_domain:bind{name = name, ["local"] = islocal}
    insert_domain:exec()
    return db:last_insert_rowid()
end
