
local connections = {}

local function add_listener(event_id, listener_function)

    local listeners = event[event_id]

    if not listeners then
        return
    end
    
    listeners[#listeners + 1] = listener_function
    
    connections[#connections + 1] = {listeners, #listeners}
    return #connections
end

local function remove_listener(connection_id)
    local connection = connections[connection_id]
    if not connection then return end
    table.remove(connection[1], connection[2])
    connections[connection_id] = nil
end

local function clear_listeners()
    for connection_id in pairs(connections) do
        remove_listener(connection_id) 
    end
end

local function create_event(event_id)
    
    event[event_id] = {}
    
    return function(...)
        local listeners = event[event_id]
        for _, listener in pairs(listeners) do
            listener(unpack(arg))
        end
    end
end

local function destroy_event(event_id)
    event[event_id] = nil
end

server.event_handler = add_listener
server.cancel_handler = remove_listener
server.cancel_handlers = clear_listeners

server.create_event_signal = create_event
server.cancel_event_signal = destroy_event

on = add_listener

