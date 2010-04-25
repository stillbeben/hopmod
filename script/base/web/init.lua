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

load_once("script/base/web/http_utils.lua")
load_once("script/base/web/http_response.lua")
load_once("script/base/web/http_request.lua")
load_once("script/base/web/login.lua")
load_once("script/base/web/admin.lua")
load_once("script/base/web/serverexec.lua")
load_once("script/base/web/listener.lua")
load_once("script/base/web/player_info.lua")
load_once("script/base/web/team_info.lua")
load_once("script/base/web/queryvars.lua")

local function unload()
    server.stop_http_server()
    http_server_root = nil
end

return {unload = unload}
