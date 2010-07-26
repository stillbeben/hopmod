local clans = {}

function clan(description)

    validate_table(description, {name="string", tag_pattern="string"})
    
    if description.auth then
        
        local server_id = auth.directory.make_server_id(description.auth.server[1])
        
        auth.directory.server{
            id = server_id,
            hostname = description.auth.server[1],
            port = description.auth.server[2]
        }
        
        auth.directory.domain{
            server = server_id,
            id = description.auth.domain
        }
        
        description.auth_domain = description.auth.domain
        description.server = nil
        
    else
        
        if description.auth_domain then
            if not auth.directory.get_domain(description.auth_domain) then
                error(string.format("Unknown auth domain for clan tag protection entry '%s'.", description.name))
            end
        else
            error(string.format("Missing auth information for clan tag protection entry '%s'.", description.name))
        end
    
    end
    
    description.tag_pattern = string.upper(description.tag_pattern)
    
    table.insert(clans, description)
end

local failure_message = {}
failure_message[auth.request_status.REQUEST_FAILED] = "You are using a clan tag in your name which is reserved by the clan '%s'."
failure_message[auth.request_status.CHALLENGE_FAILED] = "The server failed to authenticate you for the use of the reserved clan tag."
failure_message[auth.request_status.RESPONSE_FAILED] = failure_message[auth.request_status.CHALLENGE_FAILED]
failure_message[auth.request_status.TIMEOUT] = failure_message[auth.request_status.CHALLENGE_FAILED]

local function check_name(cn)
    
    local name = string.upper(server.player_name(cn))
    
    for _, clan in pairs(clans) do
        if string.match(name, clan.tag_pattern) then
            
            auth.send_request(cn, clan.auth_domain, function(cn, user_id, domain, status)
                
                if status == auth.request_status.SUCCESS then
                    server.log(string.format("%s(%i) authenticated as '%s' to use reserved clan tag.", server.player_name(cn), cn, user_id))
                end
                
                if status == auth.request_status.SUCCESS or status == auth.request_status.CANCELLED then return end
                
                server.player_msg(cn, red(string.format(failure_message[status], clan.name)))
                
                server.player_msg(cn, "You have 10 seconds to rename")
                
                local session_id = server.player_sessionid(cn)
                local old_name = server.player_name(cn)
                
                server.sleep(1000*10, function()
                    
                    if server.player_sessionid(cn) ~= session_id then return end
                    
                    if server.player_name(cn) == old_name then
                       server.kick(cn, 0, "server", "use of reserved clan tag") 
                    end
                end)
                
            end)
        end
    end
end

server.event_handler("connect", check_name)
server.event_handler("rename", check_name)

if server.file_exists("conf/clans.lua") then
    script("conf/clans.lua")
end

