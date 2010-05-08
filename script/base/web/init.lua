require "http_server"

server.start_http_server("0.0.0.0", server.serverport + 3)

local root = {}

http_server.set_root(http_server.resource({
    resolve = function(name)
        return root[name]
    end
}))

root.static = http_server.filesystem_resource("script/base/web/public", "index.htm")
http_server_root = root

load_once("script/base/web/http/utils.lua")
load_once("script/base/web/http/response.lua")
load_once("script/base/web/http/request.lua")
load_once("script/base/web/resource/login.lua")
load_once("script/base/web/resource/admin.lua")
load_once("script/base/web/resource/listener.lua")
load_once("script/base/web/resource/serverexec.lua")
load_once("script/base/web/resource/queryvars.lua")
load_once("script/base/web/resource/calls.lua")
load_once("script/base/web/resource/player_info.lua")
load_once("script/base/web/resource/team_info.lua")
load_once("script/base/web/resource/netstats.lua")

local function unload()
    server.stop_http_server()
    http_server_root = nil
end

return {unload = unload}
