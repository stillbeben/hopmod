local cmd_persist = {}

function cmd_persist.error_msg(error_msg_cn)
    server.player_msg(error_msg_cn,red("usage: #persist 0|1"))
end

function cmd_persist.playercmd(persist_cn,persist_option)
    if not persist_option then
        cmd_persist.error_msg(persist_cn)
    elseif tonumber(persist_option) == 1 then
        server.reassignteams = 0
        server.player_msg(persist_cn,orange("reshuffle teams at mapchange disabled"))
    elseif tonumber(persist_option) == 0 then
        server.reassignteams = 1
        server.player_msg(persist_cn,orange("reshuffle teams at mapchange enabled"))
    else
        cmd_persist.error_msg(persist_cn)
    end
end

if server.persist_command_master == 1 then
    function server.playercmd_persist(persist_cn,persist_option)
	return mastercmd(function()
    	    cmd_persist.playercmd(persist_cn,persist_option)
        end,persist_cn)
    end
else
    function server.playercmd_persist(persist_cn,persist_option)
	return admincmd(function()
    	    cmd_persist.playercmd(persist_cn,persist_option)
        end,persist_cn)
    end
end
