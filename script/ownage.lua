
local killingspree_message = {}
killingspree_message[5] = "\f2%s is on a \f6KILLING SPREE!!"
killingspree_message[10] = "\f2%s is on a \f6RAMPAGE!!"
killingspree_message[15] = "\f2%s is \f6DOMINATING!!"
killingspree_message[20] = "\f2%s is \f6UNSTOPPABLE!!"
killingspree_message[30] = "\f2%s is \f6GODLIKE!!"

server.event_handler("frag", function(target_cn, actor_cn)
    
    local actor_vars = server.player_vars(actor_cn)
    local actor_killingspree = actor_vars.killingspree
    
    if actor_cn ~= target_cn then
        
        actor_killingspree = (actor_killingspree or 0) + 1
        actor_vars.killingspree = actor_killingspree
        
        if actor_vars.lastkill and server.gamemillis - actor_vars.lastkill < 2000 then
            
            local actor_multikills = (actor_vars.multikills or 0) + 1
            actor_vars.multikills = actor_multikills
            
            if actor_multikills == 2 then
                server.player_msg(actor_cn, yellow("You scored a ") .. orange("DOUBLE KILL!!"))
            elseif actor_multikills == 3 then
                server.player_msg(actor_cn, yellow("You scored a ") .. orange("TRIPLE KILL!!"))
            elseif actor_multikills > 3 then
                server.player_msg(actor_cn, yellow("You scored ") ..  orange(string.format("MULTPLE KILLS(%i)!!",actor_multikills)))
            end
        end
        
        actor_vars.lastkill = server.gamemillis
        actor_vars.multikills = 0
    
    else
        actor_killingspree = 0
    end
    
    local target_vars = server.player_vars(target_cn)
    target_vars.killingspree = 0
    
    if killingspree_message[actor_killingspree] then
        server.msg(string.format(killingspree_message[actor_killingspree], server.player_name(actor_cn)))
    end
    
end)

server.event_handler("suicide", function(cn)
    
    server.player_vars(cn).killingspree = 0
    
end)
