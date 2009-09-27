
require("net")

local function resolve(hostname)
    local co = coroutine.running()
    net.async_resolve(hostname, function(ips)
        coroutine.resume(co, ips)
    end)
    return coroutine.yield(co)
end

net.resolve = resolve

function net.thread(socket, threadFunction)
    
    local derivedSocket = {}
    
    if socket.async_accept then
        
        local function accept(unused)
            local co = coroutine.running()
            socket:async_accept(function(client, errmsg)
                coroutine.resume(co, client, errmsg)
            end)
            return coroutine.yield(co)
        end
        
        derivedSocket.accept = accept
    end
    
    if socket.async_read_until then
        
        local function read_until(unused, delim)
            local co = coroutine.running()
            socket:async_read_until(delim, function(output, errmsg)
                coroutine.resume(co, output, errmsg)
            end)
            return coroutine.yield(co)
        end
        
        derivedSocket.read_until = read_until
    end
    
    if socket.async_read then
        
        local function _read(unused, buffer, readsize)
            local co = coroutine.running()
            
            local function handler(buffer, errmsg)
                coroutine.resume(co, buffer, errmsg)
            end
            
            if readsize then
                socket:async_read(buffer, readsize, handler)
            else
                socket:async_read(buffer, handler)
            end
            
            return coroutine.yield(co)
        end
        
        derivedSocket.read = _read
    end
    
    if socket.async_send then
        
        local function send(unused, data)
            local co = coroutine.running()
            socket:async_send(data, function(errmsg) 
                coroutine.resume(co, errmsg)
            end)
            return coroutine.yield(co)
        end
    
        derivedSocket.send = send
    end
    
    if socket.async_connect then
        
        local function connect(unused, ip, port)
            local co = coroutine.running()
            socket:async_connect(ip, port, function(errmsg)
                coroutine.resume(co, errmsg)
            end)
            return coroutine.yield(co)
        end
        
        derivedSocket.connect = connect
    end
    
    derivedSocket.super = socket
    
    derivedSocket.close = function(unused) socket:close() end
    derivedSocket.set_option = function(unused, name, value) socket:set_option(name, value) end
    derivedSocket.get_option = function(unused, name) return socket:get_option(name) end
    
    return coroutine.wrap(threadFunction)(derivedSocket)
end
