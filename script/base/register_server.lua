require "net"

local UPDATE = 60*60*1000

local function register_server(hostname, port, gameport, callback)

    local client = net.tcp_client()
    
    if #server.serverip > 0 then
        client:bind(server.serverip, 0)
    end
    
    client:async_connect(hostname, port, function(error_message)
        
        if error_message then
            close_connection(client, callback, error_message)
            return
        end
        
        client:async_send(string.format("regserv %i\n", gameport), function(error_message)
            
            if error_message then
                close_connection(client, callback, error_message)
                return
            end
            
            local function masterinput()
                client:async_read_until("\n", function(line, error_message)
                    
                    if not line then
                        client:close()
                        callback(error_message or "failed to read reply from server")
                        return
                    end
                    
                    local command, arg1 = line:match("([^ ]+)([ \n]*.*)\n")
                    
                    if command == "succreg" then
                        callback()
                    elseif command == "failreg" then
                        callback(arg1 or "master server rejected registration")
                    elseif command == "addgban" then
                        local ip = string.sub(arg1, 2, string.len(arg1))
                        server.ban(ip, -1, "gban")
                    elseif command == "cleargbans" then
                    else
                        callback("master server sent unknown reply")
                    end
                    masterinput()
                end)
            end
            masterinput()
            server.sleep(1000, function() client:close() end)
        end)
    end)
end

local function update()

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

server.interval(UPDATE, update)
update()
