--[[

	Function to get http-content of a webserver
	Function: http_client(host, port, get, callback, [timeout])
    
    MAKE SURE SERVER SENDS CONTENT-LENGTH, IF YOU WANT TO RECEIVE CONTENT WITHOUT A TIMEOUT!

	Copyright (C) 2010 Thomas

]]

function http_client(host, port, get, callback, timeout)
	if type(callback) ~= "function" then
		callback(nil, "error: callback must be a function!")
	end
	if host == "" or port == "" or get == "" then
		callback(nil, "error: nil parameter was given")
	end
	
	http_client_data = ""
	http_client_read_timeout = timeout
	http_client_callback = callback
	http_client_sock = net.tcp_client()
	
	function http_client_cleanup()
		http_client_sock:close()
		http_client_sock = nil
		http_client_callback = nil
		http_client_data = nil
		http_client_cleanup = nil
		http_client_read_timeout = nil
		http_client_readloop = nil
		http_client_buf = nil
		http_client_content_length = nil
	end
	
	net.async_resolve(host, function(ips)
		if not ips then error(".") end
		http_client_sock:async_connect(ips[1], port, function(errmsg)
			if errmsg then
				error(errmsg)
				http_client_cleanup()
				return
			end
			
			if not http_client_sock then return end
			
			http_client_sock:async_send("GET /" .. get .. " HTTP/1.1\r\nHost: " .. host .. "\r\n\r\n", function(errmsg, len)
				if errmsg then
					error(errmsg)
					http_client_cleanup()
					return
				end
			end)
			
			if not http_client_sock then return end
			
			if http_client_read_timeout then
			
				server.sleep(http_client_read_timeout, function()
					http_client_callback(http_client_data)
					http_client_cleanup()
				end)
				
				http_client_skipped_header = 0	
				
				http_client_sock:async_read_until("\r\n\r\n", function(data) end)
				
				function http_client_readloop() 
					http_client_sock:async_read_until("\n", function(data, errmsg)
						if errmsg or not data then
							return
						end
						http_client_data = string.format("%s%s", http_client_data, data)
						http_client_readloop()
					end)
				end
				
				http_client_readloop()
				
			else
			
				http_client_content_length = ""
				http_client_sock:async_read_until("\r\n\r\n", function(data) 
					local tmp = string.split(string.lower(data), "[^\n]+")
					for index, line in ipairs(tmp) do
						if string.find(line, "length:") then
							local i = string.len("length:") -- :P
							while (i < #line) do
								local char = string.sub(line, i, i)
								if tonumber(char) then
									http_client_content_length = http_client_content_length .. char
								end
								i = i + 1
							end
							http_client_content_length = tonumber(http_client_content_length)
							http_client_buf = net.buffer(http_client_content_length)
							http_client_sock:async_read(http_client_buf, http_client_buf:size(), function()
								http_client_callback(http_client_buf:to_string())
								http_client_cleanup()
							end)
						end
					end	
				end)
			end
			
			return 
		end)
	end)
end