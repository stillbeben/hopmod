require "http_server"
require "Json"

local listeners = {}

function argToArray(arg)
    local result = {}
    for i = 1, arg.n do result[i] = arg[i] end
    return result
end

local function createListener(events)
    
    local nextId = #listeners + 1
    local listener = {}
    
    listener.handlers = {}
    listener.queue = {}
    listener.request = nil
    
    local function dequeueEvents()
        
        if not listener.request or #listener.queue == 0 then
            return
        end
        
        http_response.send_json(listener.request, listener.queue)
        
        listener.queue = {}
        listener.request = nil
    end
    
    for _, eventName in ipairs(events) do
        
        local handler = server.event_handler_object(eventName, function(...)
            table.insert(listener.queue, {name = eventName, args = argToArray(arg)})
            dequeueEvents()
        end)
        
        table.insert(listener.handlers, handler)
    end
    
    listener.resource = http_server.resource({
        get = function(request)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            if listener.request then
                http_response.send_error(listener.request, 503, "request has been overloaded")
            end
            
            listener.request = request
            
            dequeueEvents()
        end
    })
    
    listeners[nextId] = listener
    return nextId
end

http_server_root["listener"] = http_server.resource({

    resolve = function(name)
        local listener = listeners[tonumber(name)]
        if not listener then return nil end
        return listener.resource
    end,
    
    post = function(request)
        
        request:async_read_content(function(content)
        
            if web_admin.require_backend_login(request) then
                return
            end
            
            local events = Json.Decode(content)
            local id = createListener(events)
            
            http_response.send_json(request, {listenerURI = "/listener/" .. id})
        end)
    end
})
