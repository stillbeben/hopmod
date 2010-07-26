
require "net"

local update = 3600000

local interval_active = true

local function complete(client, callback, errmsg)
    client:close()
    callback(errmsg)    
end

local function register_server(hostname, port, gameport, callback)

    local client = net.tcp_client()
    
    if #server.serverip > 0 then
        client:bind(server.serverip, 0)
    end
    
    client:async_connect(hostname, port, function(errmsg)
        
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
                
                local command, reason = line:match("([^ ]+)([ \n]*.*)\n")

                if command == "succreg" then
                    
                    complete(client, callback)
                    
                elseif command == "failreg" then
                     
                    complete(client, callback, reason or "master server rejected registration")
                    
                else
                    
                    complete(client, callback, "master server sent unknown reply")
                end
                
            end)
        end)
    end)
    
end

local function update_now()
    
    if server.publicserver == 1 then
    
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

	if interval_active == false then return -1 end

    update_now()

end)

update_now()

local function unload()
	interval_active = false
end

return {unload = unload}
