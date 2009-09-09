
local function usage_msg(error_msg_cn)
    server.player_msg(error_msg_cn, red("usage: #persist 0|1"))
end

return function(persist_cn,persist_option)

    if not persist_option then
    
        usage_msg(persist_cn)
        
    elseif tonumber(persist_option) == 1 then
    
        server.reassignteams = 0
        server.player_msg(persist_cn, orange("reshuffle teams at mapchange disabled"))
        
    elseif tonumber(persist_option) == 0 then
    
        server.reassignteams = 1
        server.player_msg(persist_cn, orange("reshuffle teams at mapchange enabled"))
         
    else
        usage_msg(persist_cn)
    end
    
end
