require "http_server"

server.start_http_server("0.0.0.0", server.serverport + 3)

load_once("script/base/web/http_utils.lua")
load_once("script/base/web/http_response.lua")
load_once("script/base/web/http_request.lua")
load_once("script/base/web/login.lua")
load_once("script/base/web/admin.lua")
load_once("script/base/web/serverexec.lua")
load_once("script/base/web/listener.lua")
load_once("script/base/web/player_info.lua")

http_server.bind_filesystem_path("static", "script/base/web/public", "index.htm")

local function unload()
    server.stop_http_server()
end

return {unload = unload}
