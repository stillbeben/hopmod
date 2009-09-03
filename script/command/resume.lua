local cmd_resume = {}

function cmd_resume.playercmd()
    server.pausegame(false)
end

if server.resume_command_master == 1 then
    function server.playercmd_resume(cn)
	return mastercmd(function()
    	    cmd_resume.playercmd()
	end,cn)
    end
else
    function server.playercmd_resume(cn)
	return admincmd(function()
    	    cmd_resume.playercmd()
	end,cn)
    end
end
