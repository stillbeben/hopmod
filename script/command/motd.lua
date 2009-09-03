-- #motd "<text>"

local cmd_motd = {}

function cmd_motd.playercmd(cn,text)
    if not text then
        server.player_msg(cn,red("#motd \"<text>\""))
    end
    server.motd = text
    server.player_msg(cn,"MOTD changed to " .. text)
end

if server.motd_command_master == 1 then
    function server.playercmd_motd(cn, text)
	return mastercmd(function()
    	    cmd_motd.playercmd(cn,text)
	end,cn)
    end
else
    function server.playercmd_motd(cn, text)
	return admincmd(function()
    	    cmd_motd.playercmd(cn,text)
	end,cn)
    end
end
