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
    [1] = "%s",
    [2] = "./script/%s",
    [3] = "./script/%s.lua",
    [4] = "./script/%s.cs",
    [5] = "./conf/%s",
    [6] = "./conf/%s.lua",
    [7] = "./conf/%s.cs"
}

function server.script(filename)
    
    for i, path in ipairs(script_paths) do
        
        local candidateFilename = string.format(path, filename)
        
        if server.file_exists(candidateFilename) then
            filename = candidateFilename
            break
        end
    end
    
    if loaded_scripts[filename] then
        server.log_error(string.format("Already loaded script \"%s\".", filename))
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

function server.unload_module(name)
    
    local control = loaded_modules[name]
    
    if not control then error(string.format("Module \"%s\" not found", name)) end
    if not control.unload then error(string.format("Module \"%s\" cannot be unloaded", name)) end
    
    control.unload()
    loaded_modules[name] = nil
    loaded_scripts[control.filename] = nil
end

server.event_handler("started", load_modules_now)
