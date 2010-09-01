--[[
    Simple HTTP Client
    Copyright (C) 2010 Graham Daws
]]
local net = require "net"
local url = require "url"
local string = require "string"
local tonumber = tonumber

module "http.client"

local MAX_REDIRECTS = 5

function unfold_headers(headers)
    headers = string.gsub(headers, "\r\n%s+", " ")
    return headers
end

function parse_headers(headers)
    local header = {}
    for name, value in string.gmatch(headers, "([^:\r\n]+): *([^\r\n]+)") do
        header[string.lower(name)] = value
    end
    return header
end

local function read_status_line(socket, callback)
    socket:async_read_until("\r\n", function(status_line, error_message)
        if error_message then
            callback(nil, nil, {socket_error = error_message})
            return
        end
        local version, status_code = string.match(status_line, "HTTP/(%d.%d) (%d%d%d)")
        if not version or not status_code then
            callback(nil, nil, {parse_error = "malformed status line"})
            return
        end
        callback(version, status_code)
    end)
end

local function read_headers(socket, callback)
    socket:async_read_until("\r\n\r\n", function(headers, error_message)
        if error_message then
            callback(nil, error_message)
            return
        end
        callback(parse_headers(unfold_headers(headers)))
    end)
end

local function read_body(socket, headers, callback)

    local content_length = tonumber(headers["content-length"])
    
    if not content_length then
        callback(nil, {http_error = "malformed content-length field"})
        return
    end
    
    local buffer = net.buffer(content_length)
    
    socket:async_read(buffer, function(error_message)
        if error_message then
            callback(nil, {socket_error = error_message})
            return
        end
        callback(buffer:to_string())
    end)
end

local function read_trailer_headers(socket, callback)
    socket:async_read_until("\r\n", function(line, error_message)
        
        if error_message then
            callback({socket_error = error_message})
            return
        end
        
        if line == "\r\n" then
            callback(nil)
        else
           read_trailer_headers(socket, callback) 
        end
    end)
end

local function read_chunked_body(socket, header, callback, body)

    body = body or ""
    
    socket:async_read_until("\r\n", function(line, error_message)
    
        if error_message then
            callback(nil,{socket_error = error_message})
            return
        end
        
        local chunk_size = tonumber(string.match(line, "^[%dabcdef]+"), 16)
        
        if chunk_size == 0 then
            
            read_trailer_headers(socket, function(errors)
                
                if errors then
                    callback(nil, errors)
                    return
                end
                
                callback(body)
            end)
            
            return
        end
        
        local buffer = net.buffer(chunk_size)
        
        socket:async_read(buffer, function(error_message)
        
            if error_message then
                callback(nil,{socket_error = error_message})
                return
            end
            
            body = body .. buffer:to_string()
            
            socket:async_read_until("\r\n", function(empty_line, error_message)
                
                -- TODO check empty_line
                 
                if error_message then
                    callback(nil,{socket_error = error_message})
                    return
                end
                
                read_chunked_body(socket, header, callback, body)
            end)
        end)
    end)
end

local body_readers = {
    identity = read_body,
    chunked = read_chunked_body
}

local function is_redirect(status_code)
    if string.sub(status_code,1,1) ~= "3" then return false end
    return status_code == "301" or status_code == "302" or status_code == "303"
end 

function get(resource, callback, state)
    
    state = state or {redirects = 0}
    
    resource = url.parse(resource)
    
    if resource.scheme ~= "http" or not resource.host then
        callback(nil,{http_error = "invalid url"})
        return
    end
    
    resource.path = resource.path or "/"
    
    local client = net.tcp_client()
    
    client:async_connect(resource.host, resource.port or 80, function(error_message)
        
        if error_message then
            callback(nil, {socket_error = error_message})
            return
        end
        
        local host_field = resource.host
        if resource.port then
            host_field = host_field .. ":" .. resource.port
        end
        
        local request = "GET " .. url.build_path(url.parse_path(resource.path)) .. " HTTP/1.1\r\nHost: " .. host_field .. "\r\nConnection: close\r\n\r\n"
        
        client:async_send(request, function(error_message)

            if error_message then
                callback(nil, {socket_error = error_message})
               return
            end
            
            read_status_line(client, function(version, status_code, errors)
                
                if errors then
                    callback(nil, errors)
                    return
                end
                
                read_headers(client, function(headers, errors)
                    
                    if errors then
                        callback(nil, errors)
                        return
                    end
                    
                    if is_redirect(status_code) then
                    
                        state.redirects = state.redirects + 1
                        
                        if state.redirects > MAX_REDIRECTS then
                            callback(nil, {http_error = "too many redirects"})
                            return
                        end
                        
                        get(headers["location"], callback, state)
                    end
                    
                    local read_body = body_readers[headers["transfer-encoding"] or "identity"]
                    
                    if not read_body then
                        callback(nil,{http_error = string.format("client doesn't support %s transfer encoding", headers["transfer-encoding"])})
                        return
                    end
                    
                    read_body(client, headers, function(body, errors)
                        
                        if errors then
                            callback(nil, errors)
                            return
                        end
                        
                        callback(body, {
                            version = version,
                            response_status = status_code,
                            headers = headers
                        })
                    end)
                end)
            end)
        end)
    end)
end

