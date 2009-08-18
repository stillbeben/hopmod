local gamevars = {}
local pvars = {}

local function accessvar(vartab, cn, name, value, defval)
    
    vartab[server.player_id(cn)] = vartab[server.player_id(cn)] or {}
    
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
    vartab[server.player_id(cn)] = vartab[server.player_id(cn)] or {}
    vartab[server.player_id(cn)][name] = nil
end

function server.player_var(cn, name, value, defval)
    return accessvar(gamevars, cn, name, value, defval)
end

function server.player_pvar(cn, name, value, defval)
    return accessvar(pvars, cn, name, value, defval)
end

function server.player_hasvar(cn, name)
    return hasvar(gamevars, cn, name)
end

function server.player_haspvar(cn, name)
    return hasvar(pvars, cn, name)
end

function server.player_unsetvar(cn, name)
    return unsetvar(gamevars, cn, name)
end

function server.player_unsetpvar(cn, name)
    return unsetvar(pvars, cn, name)
end

server.event_handler("mapchange", function()
    gamevars = {}
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
    local tab = pvars[server.player_id(cn)]
    if not tab then
        tab = {}
        pvars[server.player_id(cn)] = tab
    end
    return tab
end
