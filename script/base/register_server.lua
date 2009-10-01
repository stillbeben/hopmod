
require "net"

local update = 3600000

local function complete(client, callback, errmsg)
    client:close()
    callback(errmsg)    
end

local function register_server(hostname, port, gameport, callback)

    net.async_resolve(hostname, function(addresses)
        
        if not addresses then
            callback("could not resolve hostname " .. hostname)
            return 
        end
        
        local client = net.tcp_client()
        
        client:async_connect(addresses[1], port, function(errmsg)
            
            if errmsg then
                complete(client, callback, errmsg)
                return
            end
            
            client:async_send(string.format("regserv %i\n", gameport), function(errmsg)
                
                if errmsg then
                    complete(client, callback, errmsg)
                    return
                end
                
                client:async_read_until("\n", function(line, errmsg)
                    
                    if not line then
                        complete(client, callback, errmsg or "failed to read reply from server")
                        return
                    end
                    
                    local arguments = line:split("[^ \n]+")
                    
                    if arguments[1] == "succreg" then
                        
                        complete(client, callback)
                        
                    elseif arguments[1] == "failreg" then
                        
                        -- Learn more about regular expressions so I don't have to do stuff like this
                        local failmsg = ""
                        for i,v in ipairs(arguments) do
                            if i > 1 then failmsg = failmsg .. v .. " " end
                        end
                        
                        complete(client, callback, failmsg or "master server rejected registration")
                        
                    else
                        
                        complete(client, callback, "master server sent unknown reply")
                    end
                    
                end)
            end)
        end)
    end)
end

local function update_now()
    
    if server.updatemaster == 1 then
    
        register_server(server.masterserver, server.masterserverport, server.serverport, function(error_message)
            
            if error_message then
                server.log_error("Master server error: " .. error_message)
            else
                server.log_status("Server registration succeeded.")
            end
        end)
       
    end
end

server.interval(update, function()
    update_now()
end)

update_now()
