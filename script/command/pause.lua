local cmd_pause = {}

function cmd_pause.playercmd()
    server.pausegame(true)
end

if server.pause_command_master == 1 then
    function server.playercmd_pause(cn)
	return mastercmd(cmd_pause.playercmd,cn)
    end
else
    function server.playercmd_pause(cn)
	return admincmd(cmd_pause.playercmd,cn)
    end
end
