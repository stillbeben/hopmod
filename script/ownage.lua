local first_frag_events_active = 0
local first_frag_events = {}

local function first_frag_disabler()
    if first_frag_events_active == 1 then
	first_frag_events_active = 0
	
	for a,b in ipairs(first_frag_events) do
	    server.cancel_handler(b)
	end
    end
end

local function first_frag_event(tcn,acn)
    server.msg(string.format("\f2%s opened the \f6FIGHT!!",server.player_name(acn)))
    first_frag_disabler()
end

local function first_frag_enabler()
    if first_frag_events_active == 0 then
	first_frag_events_active = 1
	
	first_frag_events = {}
	local event_frag = server.event_handler("frag",first_frag_event)
	table.insert(first_frag_events,event_frag)
    end
end

server.event_handler("finishedgame",function()
    first_frag_enabler()
end)

first_frag_enabler()



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
    else
        actor_killingspree = 0
        actor_vars.is_killingspree = 0
    end
    
    if killingspree_message[actor_killingspree] then
	actor_vars.is_killingspree = 1
	server.msg(string.format(killingspree_message[actor_killingspree], server.player_name(actor_cn)))
    end
    
    if server.player_vars(target_cn).is_killingspree == 1 then
	server.msg(string.format("\f2%s stopped \f6%s!!",server.player_name(actor_cn),server.player_name(target_cn)))
	server.player_vars(target_cn).is_killingspree = 0
    end
    server.player_vars(target_cn).killingspree = 0
end)

server.event_handler("suicide", function(cn)
    server.player_vars(cn).killingspree = 0
end)



server.event_handler("frag", function(target_cn, actor_cn)
    local actor_vars = server.player_vars(actor_cn)
    
    if actor_cn ~= target_cn then
        if actor_vars.lastkill and server.gamemillis - actor_vars.lastkill < 2000 then
            local actor_multikills = (actor_vars.multikills or 1) + 1
            actor_vars.multikills = actor_multikills
            
            if actor_multikills == 2 then
                server.player_msg(actor_cn, yellow("You scored a ") .. orange("DOUBLE KILL!!"))
            elseif actor_multikills == 3 then
                server.player_msg(actor_cn, yellow("You scored a ") .. orange("TRIPLE KILL!!"))
            elseif actor_multikills > 3 then
                server.player_msg(actor_cn, yellow("You scored ") ..  orange(string.format("MULTPLE KILLS(%i)!!",actor_multikills)))
            end
        else
	    actor_vars.multikills = 1
        end
	
        actor_vars.lastkill = server.gamemillis
    else
	actor_vars.multikills = 1
    end
    
    server.player_vars(target_cn).multikills = 1
end)

server.event_handler("suicide", function(cn)
    server.player_vars(cn).multikills = 1
end)
