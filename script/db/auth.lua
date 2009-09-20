
local using_sqlite = (server.auth_use_sqlite == 1)
local db, perr, search_for_domain, select_local_domains, is_domain_local, search_for_user, select_domain_users, insert_user, insert_domain
local domains -- only used when not using sqlite

if using_sqlite then

    require "sqlite3"
    require "sqlite3utils"

    db = sqlite3.open(server.auth_db_filename)
    sqlite3utils.createMissingTables("./script/db/auth_schema.sql", db)
    
    search_for_domain,perr = db:prepare("SELECT * FROM domains WHERE name = :name")
    if not search_for_domain then error(perr) end

    select_local_domains, perr = db:prepare("SELECT * FROM domains WHERE local = 1")
    if not select_local_domains then error(perr) end

    is_domain_local, perr = db:prepare("SELECT local FROM domains WHERE id = :domain_id")
    if not is_domain_local then error(perr) end

    search_for_user, perr = db:prepare("SELECT * FROM users WHERE domain_id = :domain_id and name = :name")
    if not search_for_user then error(perr) end

    select_domain_users, perr = db:prepare("SELECT * FROM users WHERE domain_id = :domain_id")
    if not select_domain_users then error(perr) end

    insert_user, perr = db:prepare("INSERT INTO users (domain_id, name, pubkey) VALUES (:domain_id,:name,:pubkey)")
    if not insert_user then error(perr) end

    insert_domain, perr = db:prepare("INSERT INTO domains (name, local) VALUES (:name,:local)")
    if not insert_domain then error(perr) end
    
    server.event_handler("shutdown", function()
        if db then db:close() end
    end)
    
    server.close_auth_db = function()
        db:close()
        db = nil
    end
    
else
    domains = {}
end

local auth_request = {}

auth_domain_handlers = {}
auth = {}
auth.authserver_offline = false

server.event_handler("authreq", function(cn, name, domain)

    local req_id = server.player_authreq(cn)
    auth_request[req_id] = {}
    local req = auth_request[req_id]
    
    local row
    
    if using_sqlite then
        search_for_domain:bind{name = domain}
        row = search_for_domain:first_row()
    else
        row = domains[domain]
    end
    
    req.name = name
    
    req.player_sessid = server.player_sessionid(cn)
    
    if row then
        req.domain = domain
        req.domain_id = row.domain_id
    end
    
    if not row or row["local"] == 0 then
    
        req.delegated = true
        
        if not server.delegateauth(cn, req.domain or "") then
            
            auth.cleanup_request(cn, req_id)
            auth.authserver_offline = true
            
        else
            auth.authserver_offline = false
        end
 
    else
   
        req.delegated = false
        
        local user_row
        
        if using_sqlite then
            search_for_user:bind{domain_id = row.id, name = name}
            user_row = search_for_user:first_row()
        else
            user_row = row.users[name]
        end
        
        
        
        if user_row then
            
            local ans,chal = server.genchallenge(user_row.pubkey)
            req.answer = ans
            server.sendauthchallenge(cn, chal)
            
        else
            server.signal_auth_failure(cn, req_id)
        end
    end
    
end)

server.event_handler("authrep", function(cn, reqid, answer)

    reqid = tonumber(reqid)
    local req = auth_request[reqid]
    
    if not req then
        error("auth reply to non-existent request (id " .. tonumber(reqid) .. ")" )
    end
    
    if req.delegated then
    
        if not server.relayauthanswer(cn, answer) then
            auth.cleanup_request(cn, reqid)
            auth.authserver_offline = true
        else
            auth.authserver_offline = false
        end
        
        return
    end

    if server.checkchallenge(answer, req.answer) then
        server.signal_auth_success(cn, reqid)
    else
        server.signal_auth_failure(cn, reqid)
    end

end)

local function dispatchHandlers(domain, cn, name)

    local handlers = auth_domain_handlers[domain]
    if handlers then
        for i, handler in ipairs(handlers) do
            handler(cn, name)
        end
    end
    
end

server.event_handler("auth", function(cn, id, name, domain, success)
    
    local req_id = tonumber(id)
    local req = auth_request[req_id]
    
    if not req.delegated then
        server.freechalanswer(req.answer)
    end
    
    if success then
        if req.domain then
            
            dispatchHandlers(req.domain, cn, name)
            
        else

            local admin_present = server.master ~= -1 and server.player_priv_code(server.master) == server.PRIV_ADMIN
            
            if server.player_priv_code(cn) == 0 and not admin_present and server.using_master_auth() then
                
                if server.setmaster(cn) then
                    server.msg(string.format("%s claimed master as '%s'",server.player_name(cn),magenta(name)))
                else
                    server.player_msg(cn, red("Master is disabled."))
                end
            end
        end
    end
    
    auth.cleanup_request(cn, req_id)
    
end)

function auth.get_domain_id(name)

    if using_sqlite then
    
        search_for_domain:bind{name = name}
        local row = search_for_domain:first_row()
        if not row then return end
        return row.id
        
    else
        if domains[name] then return name else return nil end
    end
    
end

function auth.is_domain_local(domain_id)
    
    if using_sqlite then
        is_domain_local:bind{domain_id = domain_id}
        if is_domain_local:first_row()["local"] == 1 then return true else return false end
    else
        --if not domains[domain_id] then return false end
        return domains[domain_id]["local"]
    end
end

function auth.found_name(name,domain_id)
    
    if using_sqlite then

        search_for_user:bind{domain_id = domain_id, name = name}
        local row = search_for_user:first_row()
        if row then return true else return false end
    
    else
        return domains[domain_id].users[name] ~= nil
    end
end

function auth.add_user(name, pubkey, domain)
    
    if using_sqlite then

        local domain_id = auth.get_domain_id(domain)
        if not domain_id then error("domain not found") end
        if auth.found_name(name, domain_id) then error("user name already in use") end
        insert_user:bind{domain_id = domain_id, name = name, pubkey = pubkey}
        insert_user:exec()
        
    else
        domains[domain].users[name] = {pubkey = pubkey}
    end
end

function auth.add_domain(name, islocal)
    
    if using_sqlite then

        local domain_id = auth.get_domain_id(name)
        if domain_id then return domain_id end
        if not islocal then islocal = true end
        insert_domain:bind{name = name, ["local"] = islocal}
        insert_domain:exec()
        return db:last_insert_rowid()
    
    else
        domains[name] = {users = {}, ["local"] = islocal}
    end
end

function auth.enumerate_local_domains(enumerator)
    
    if not using_sqlite then error("requires sqlite") end
    
    for row in select_local_domains:rows() do
        enumerator(row)
    end
end

function auth.enumerate_domain_users(domain_id, enumerator)

    if not using_sqlite then error("requires sqlite") end
    
    select_domain_users:bind{domain_id = domain_id}
    for row in select_domain_users:rows() do
        enumerator(row)
    end
end

-- used by sauer_authserver
function auth.load_users()
    auth.enumerate_local_domains(function(domain)
        auth.enumerate_domain_users(domain.id, function(user)
            server.adduser(user.name, domain.name, user.pubkey)
        end)
    end)
end
server.load_users = auth.load_users

function auth.add_domain_handler(domain, handler)
    
    -- TODO print call stack trace to find out what keeps calling this func with nil domain
    if not domain then return end
    
    local handlers = auth_domain_handlers[domain]
    local index
    
    if handlers then
        index = #handlers + 1
        handlers[index] = handler
    else
        index = 1
        auth_domain_handlers[domain] = {}
        auth_domain_handlers[domain][index] = handler
    end
    
    return index
end

function auth.cancel_domain_handler(domain, index)
    if not auth_domain_handlers[domain] then error("domain not found") end
    auth_domain_handlers[domain][index] = nil
end

function auth.has_request_pending(cn, domain_name)
    
    local sessid = server.player_sessionid(cn)

    local vars = server.player_vars(cn)
    if vars.authreq_domain and vars.authreq_domain == domain_name then
        return true
    end
    
    for id, req in pairs(auth_request) do
        if req.sessid == sessid and req.domain_name == domain_name then 
            return true
        end
    end
    
    return false
end

function auth.cleanup_request(cn, req_id)
    server.player_vars(cn).authreq_domain = nil
    auth_request[req_id] = nil
end

function auth.sendauthreq(cn, domain_name)

    if not auth.has_request_pending(cn, domain_name) then
        server.sendauthreq(cn, domain_name)
        server.player_vars(cn).authreq_domain = domain_name -- not a full-proof way of detecting duplicates
    end
    
end

server.auth_add_user = auth.add_user
server.auth_add_domain = auth.add_domain
server.auth_is_domain_local = auth.is_domain_local
