require "persistence"

local IPMASK_VARS_FILE = "log/player_vars"

local ipmask_vars = {}
local ipmask_vars_by_ipmask = net.ipmask_table()
local ipmask_vars_by_name = {}
local ipmask_vars_store_on_update = true

local player_session_vars = {}

local function store_vars()
    persistence.store(IPMASK_VARS_FILE .. ".tmp", ipmask_vars, ipmask_vars_by_name)
    os.rename(IPMASK_VARS_FILE .. ".tmp", IPMASK_VARS_FILE)
end

server.store_ipvars = store_vars

local function load_vars()

    ipmask_vars, ipmask_vars_by_name = persistence.load(IPMASK_VARS_FILE)
    
    if not ipmask_vars then
        ipmask_vars = {}
        ipmask_vars_by_name = {}
    end
    
    ipmask_vars_by_ipmask = net.ipmask_table()
    -- Load ipmask_vars_by_ipmask index
    for ipmask_string, vars in pairs(ipmask_vars) do
       ipmask_vars_by_ipmask[ipmask_string] = vars 
    end
end

load_vars()

function server.store_ipvars_on_update(yes)
    if yes then
        ipmask_vars_store_on_update = true
    else
        ipmask_vars_store_on_update = false
    end
end

function server.set_ip_var(ipmask, name, value)

    ipmask = net.ipmask(ipmask):to_string() -- Normalized ipmask string representation
    
    local vars = ipmask_vars[ipmask]
    if not vars then
        vars = {}
        ipmask_vars[ipmask] = vars
        ipmask_vars_by_ipmask[ipmask] = vars
    end
    
    -- Update ipmask_vars_by_name index
    local existing_value = vars[name]
    if not existing_value then
        local instances_array = ipmask_vars_by_name[name]
        if value then
            if not instances_array then
                instances_array = {}
                ipmask_vars_by_name[name] = instances_array
            end
            instances_array[#instances_array + 1] = {ipmask, value}
        else
            if instances_array then
                for index, instance in pairs(instances_array) do
                    if instance[1] == ipmask then
                        table.remove(instances_array, index) -- FIXME not working
                    end
                end
            end
        end
    end
    
    vars[name] = value
    
    -- Remove the ipmask key when the vars table becomes empty
    if not value then
        if next(vars) == nil then
            ipmask_vars[ipmask] = nil
            ipmask_vars_by_ipmask[ipmask] = nil   
        end
    end
    
    if ipmask_vars_store_on_update then
        store_vars()
    end
end

function server.ip_vars(ipmask)
    
    if not ipmask then
        return ipmask_vars
    end
    
    local matches = ipmask_vars_by_ipmask[ipmask]
    local vars = {}
    for _, match in ipairs(matches) do
        for key, value in pairs(match) do
            vars[key] = value
        end
    end
    return vars
end

function server.ip_var_instances(name)
    return ipmask_vars_by_name[name] or {}
end

function server.player_set_session_var(cn, name, value)
    local session_id = server.player_sessionid(cn)
    local session_vars = player_session_vars[session_id]
    if not session_vars then
        session_vars = {}
        player_session_vars[session_id] = session_vars
    end
    session_vars[name] = value
end

function server.player_vars(cn)

    local id = server.player_id(cn)
    if id == -1 then error("invalid cn") end
    
    local vars = server.ip_vars(server.player_ip(cn))
    
    local session_vars = player_session_vars[server.player_sessionid(cn)]
    if session_vars then
        for key, value in pairs(session_vars) do
            vars[key] = value            
        end
    end
    
    return vars
end

