local cmd_specall = {}

function cmd_specall.playercmd()
    server.specall()
end

if server.specall_command_master == 1 then
    function server.playercmd_specall(cn)
	return mastercmd(cmd_specall.playercmd,cn)
    end
else
    function server.playercmd_specall(cn)
	return admincmd(cmd_specall.playercmd,cn)
    end
end
