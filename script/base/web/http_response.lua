require "Json"

local function send_json(request, tab)
    
    local output = Json.Encode(tab)
    
    local response = http_server.response(request, 200)
    response:header("Content-Type", "application/json")
    response:set_content_length(#output)
    response:send_header()
    response:send_body(output)
end

http_response = {send_json = send_json}

return http_response
