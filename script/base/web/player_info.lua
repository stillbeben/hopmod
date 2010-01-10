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

http_server.bind("players", http_server.resource({
    get = function(request)
        http_response.send_json(request, players())
    end
}))
