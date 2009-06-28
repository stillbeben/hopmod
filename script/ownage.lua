
local killingspree_message = {}
killingspree_message[5] = "\f2%s is on a \f6KILLING SPREE!!"
killingspree_message[10] = "\f2%s is on a \f6RAMPAGE!!"
killingspree_message[15] = "\f2%s is \f6DOMINATING!!"
killingspree_message[20] = "f2%s is \f6UNSTOPPABLE!!"
killingspree_message[30] = "\f2%s is \f6GODLIKE!!"

server.event_handler("frag", function(target_cn, actor_cn)
    
    local actor_vars = server.player_vars(actor_cn)
    local actor_killingspree = actor_vars.killingspree
    
    if actor_cn ~= target_cn then
        
        actor_killingspree = (actor_killingspree or 0) + 1
        actor_vars.killingspree = actor_killingspree
    
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
