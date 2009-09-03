-- [[ based on a player command written by Thomas ]] --

local cmd_maxclients = {}

function cmd_maxclients.playercmd(cn,arg1)
    if not arg1 then
	server.player_msg(cn,red("missing maxclient argument"))
	return
    end
    if arg1 >= server.playercount and arg1 <= "128" then
        server.maxplayers = arg1
    end
end

if server.maxclients_command_master == 1 then
    function server.playercmd_maxclients(cn,arg1) 
	return mastercmd(function()
    	    cmd_maxclients.playercmd(cn,arg1)
	end,cn)
    end
else
    function server.playercmd_maxclients(cn,arg1) 
	return admincmd(function()
    	    cmd_maxclients.playercmd(cn,arg1)
	end,cn)
    end
end
