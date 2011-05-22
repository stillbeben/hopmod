local event_emitter = {}

function EventEmitter()
    local object = {}
    setmetatable(object, {__index = event_emitter})
    object._listeners = {}
    return object
end

function event_emitter:emit(event_name, ...)
    local listeners = self._listeners[event_name]
    if not listeners then return end
    for _, listener in pairs(listeners) do
        local pcall_status, error_message = pcall(listener, unpack(arg))
        if not pcall_status then
            server.log_event_error(event_name, error_message)
        end
    end
end

function event_emitter:on(event_name, callback)
    local listeners = self._listeners[event_name] or {}
    listeners[#listeners + 1] = callback
    self._listeners[event_name] = listeners
    return #listeners
end

function event_emitter:remove_listener(event_name, callback)
    
    local listeners = self._listeners[event_name]
    if not listeners then return false end
    
    if type(callback) == "number" then
        table.remove(listeners, callback)
        return
    end
    
    assert(type(callback) == "function")
    
    for _, listener in pairs(listeners) do
        if listener == callback then
            return true
        end
    end
    return false
end

function event_emitter:listeners(event_name)
    return self._listeners[event_name]
end

