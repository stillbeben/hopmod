local gamevars = {}
local permvars = {}

local function accessvar(vartab, cn, name, value, defval)
    
    local id = server.player_id(cn)
    
    vartab[id] = vartab[id] or {}
    
    if value then -- set var
    
        vartab[server.player_id(cn)][name] = value
        
        return value
        
    else -- get var
    
        return vartab[server.player_id(cn)][name] or defval
    end
    
end

local function hasvar(vartab, cn, name)
    vartab[server.player_id(cn)] = vartab[server.player_id(cn)] or {}
    if vartab[server.player_id(cn)][name] then return true else return false end
end

local function unsetvar(vartab, cn, name)
    
    local id = server.player_id(cn)
    
    vartab[id] = vartab[id] or {}
    vartab[id][name] = nil
    
end

function server.player_var(cn, name, value, defval)
    return accessvar(gamevars, cn, name, value, defval)
end

function server.player_pvar(cn, name, value, defval)
    return accessvar(permvars, cn, name, value, defval)
end

function server.player_hasvar(cn, name)
    return hasvar(gamevars, cn, name)
end

function server.player_haspvar(cn, name)
    return hasvar(permvars, cn, name)
end

function server.player_unsetvar(cn, name)
    return unsetvar(gamevars, cn, name)
end

function server.player_unsetpvar(cn, name)
    return unsetvar(permvars, cn, name)
end

server.event_handler("mapchange", function()
    gamevars = {}
end)

server.event_handler("maintenance", function()
    permvars = {}
    server.clear_player_ids()
end)

function server.player_vars(cn)

    local tab = gamevars[server.player_sessionid(cn)]
    
    if not tab then
        tab = {}
        gamevars[server.player_sessionid(cn)] = tab
    end
    
    return tab
    
end

function server.player_pvars(cn)

    local id = server.player_id(cn)
    local vars = permvars[id]
    
    if not vars then
        vars = {}
        permvars[id] = vars
    end

    return vars
    
end


