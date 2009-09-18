
player_commands = {}

server.event_handler("text", function(cn, text)
    
    local arguments = server.parse_player_command(text)
    
    if server.use_command_prefix == 1 then
        
        -- check for normal chat message
        if string.sub(text, 1, 1) ~= server.command_prefix then
            return 0
        end
        
        arguments[1] = string.sub(arguments[1],2)
    end
    
    local command = player_commands[arguments[1]]
    
    if not command then
        if server.process_player_command(cn, text) then
            return -1
        else 
            return 0
        end
    end
    
    arguments[1] = cn
    
    local privilege = server.player_priv_code(cn)
    
    if not (command.enabled == true) then
        server.player_msg(cn, red("Command disabled."))
        return -1
    end
    
    if (command.require_admin == true and privilege < server.PRIV_ADMIN) or (command.require_master == true and privilege < server.PRIV_MASTER) then
        server.player_msg(cn, red("Permission denied."))
        return -1
    end
    
    local success, errmsg = pcall(command._function, unpack(arguments))
    
    if success == false then
        
        local message = "Command error"
        
        if errmsg then
            message = message .. ": " .. errmsg .. "."
        else
            message = message .. "!"
        end
        
        server.player_msg(cn, red(message))
    end
    
    return -1
end)

local function create_command(name)
    local command = {name = name, enabled = false, require_admin = false, require_master = false, _function = nil}
    player_commands[name] = command
    return command
end

local function set_commands(commandlist, fields)
    for i, cmdname in pairs(commandlist:split("[^ \n\t]+")) do
        local command = player_commands[cmdname] or create_command(cmdname)
        
        for field_name, field_value in pairs(fields) do
            command[field_name] = field_value
        end
    end
end

function server.enable_commands(commandlist)
    return set_commands(commandlist, {enabled = true})
end

function server.disable_commands(commandlist)
    return set_commands(commandlist, {enabled = false})
end

function server.admin_commands(commandlist)
    return set_commands(commandlist, {require_admin = true})
end

function server.master_commands(commandlist)
    return set_commands(commandlist, {require_admin = false, require_master = true})
end

local function set_priv(command, priv)

    if priv == "admin" then 
        
        command.require_admin = true
        command.require_master = false
        
    elseif priv == "master" then 
        
        command.require_admin = false
        command.require_master = true
        
    end
end

local function get_new_command_table(name)
    
    local command = player_commands[name] or create_command(name)
    
    if command._function then
        server.log_error(string.format("Overwriting player command '%s'", name))
    end
    
    return command
end

function player_command_script(name, filename, priv)
    
    local command = get_new_command_table(name)
    
    set_priv(command, priv)
    
    local script,err = loadfile(filename)
    if not script then error(err) end
    
    command_info = command
    command._function = script()
    command_info = nil
end

function player_command_function(name, func, priv)

    local command = get_new_command_table(name)
    set_priv(command, priv)
    command._function = func
    
end

function player_command_alias(aliasname, sourcename)
    player_commands[aliasname] = player_commands[sourcename]
end

function log_unknown_player_commands()
    for name, command in pairs(player_commands) do
        if not command._function then
            server.log_error(string.format("No function loaded for player command '%s'", name))
        end
    end
end

function admincmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_ADMIN) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function mastercmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_MASTER) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

