local started = false
local modules = {}
local loaded_modules = {}
local loaded_scripts = {}

local script_extension_handlers = {
    lua = dofile,
    cs = server.execCubeScriptFile,
    _default = server.execCubeScriptFile
}

local script_paths = {
    [01] = "%s",
    [02] = "%s.lua",
    [03] = "%s.cs",
    [04] = "./script/%s.lua",
    [05] = "./script/%s",
    [06] = "./script/module/%s.lua",
    [07] = "./script/module/%s",
    [08] = "./script/%s.cs",
    [09] = "./conf/%s",
    [10] = "./conf/%s.lua",
    [11] = "./conf/%s.cs"
}

local function find_script(filename)
    
    local real_filename
    
    for i, path in ipairs(script_paths) do
        
        local candidateFilename = string.format(path, filename)
        
        if server.file_exists(candidateFilename) then
            real_filename = candidateFilename
            break
        end
    end
    
    return real_filename
end

server.find_script = find_script

function server.script(filename)
    
    filename = find_script(filename) or filename
    
    if loaded_scripts[filename] then
        return nil
    end
    
    local extension = string.gmatch(filename, "%.(%a+)$")() or "_default"
    local handler = script_extension_handlers[extension]
    
    if not handler then
        error(string.format("Unrecognized file extension for script \"%s\".", filename))
    end
    
    return handler(filename), filename
end

function server.load_once(filename)
    local retval, filename = server.script(filename)
    loaded_scripts[filename] = true
    return retval, filename
end

script = server.script
load_once = server.load_once

local function load_module(name)

    local control, filename = load_once(name)
    control = control or {}
    
    control.filename = filename
    loaded_modules[name] = control
end

local function load_modules_now()

    for i, name in ipairs(modules) do
        load_module(name)
    end
    
    modules = {}
    
    started = true
end

function server.module(name)

    if started == true then
        load_module(name)
    else
        table.insert(modules, name)
    end
    
end

function server.unload_module(name, hide_warnings)
    
    local control = loaded_modules[name]
    
    if not control then error(string.format("Module \"%s\" not found", name)) end
    
    if control.unload then
        control.unload()
    else
        if not hide_warnings then
            server.log_error(string.format("Module \"%s\" cannot be unloaded", name))
        end
        
        return
    end
    
    loaded_modules[name] = nil
    loaded_scripts[control.filename] = nil
    
    collectgarbage()
end

local function unload_all_modules()
    for name in pairs(loaded_modules) do 
        catch_error(server.unload_module, name, true)
    end
end

server.event_handler("started", load_modules_now)
server.event_handler("shutdown", unload_all_modules)
