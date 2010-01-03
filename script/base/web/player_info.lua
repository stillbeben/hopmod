require "http_server"
require "Json"

local function player(p)

    local output = {}
    
    output.cn = p.cn
    output.sessionid = p:sessionid()
    output.id = p:id()
    output.name = p:name()
    output.team = p:team()
    output.ip = p:ip()
    output.ping = p:ping()
    output.lag = p:lag()
    output.priv = p:priv()
    output.status = p:status()
    output.connection_time = p:connection_time()
    output.timeplayed = p:timeplayed()
    output.frags = p:frags()
    output.deaths = p:deaths()
    output.suicides = p:suicides()
    output.teamkills = p:teamkills()
    output.misses = p:misses()
    output.shots = p:shots()
    
    return output
end

local function players()
    local output = {}
    for p in server.gplayers() do
        table.insert(output, player(p))
    end
    return output
end

local function send_json(request, tab)
    
    local output = Json.Encode(tab)
    
    local response = http_server.response(request, 200)
    response:header("Content-Type", "application/json")
    response:set_content_length(#output)
    response:send_header()
    response:send_body(output)
end

http_server.bind("players", http_server.resource({
    get = function(request)
        send_json(request, players())
    end
}))
