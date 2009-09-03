local cmd_unspecall = {}

function cmd_unspecall.playercmd()
    server.unspecall()
end

if server.unspecall_command_master == 1 then
    function server.playercmd_unspecall(cn)
	return mastercmd(cmd_unspecall.playercmd(),cn)
    end
else
    function server.playercmd_unspecall(cn)
	return admincmd(cmd_unspecall.playercmd(),cn)
    end
end
