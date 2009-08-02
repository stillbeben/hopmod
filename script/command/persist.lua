-- #persist 0|1

function server.playercmd_persist(persist_cn,persist_option)
    local function persist_error_msg(persist_error_msg_cn)
        server.player_msg(persist_error_msg_cn,red("usage: #persist 0|1"))
    end
    
    return admincmd(function()
        if not persist_option then
            persist_error_msg(persist_cn)
        elseif tonumber(persist_option) == 1 then
            server.reassignteams = 0
            server.player_msg(persist_cn,orange("reshuffle teams at mapchange disabled"))
        elseif tonumber(persist_option) == 0 then
	    server.reassignteams = 1
            server.player_msg(persist_cn,orange("reshuffle teams at mapchange enabled"))
        else
            persist_error_msg(persist_cn)
        end
    end,persist_cn)
end
