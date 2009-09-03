-- [[ based on a player command written by Thomas ]] --

local cmd_reload = {}

function cmd_reload.playercmd()
    server.reloadscripts()
end

if server.reload_command_master == 1 then
    function server.playercmd_reload(cn_client) 
	return mastercmd(cmd_reload.playercmd(),cn_client)
    end
else
    function server.playercmd_reload(cn_client) 
	return admincmd(cmd_reload.playercmd(),cn_client)
    end
end

