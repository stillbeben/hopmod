local teamkill_limit = server.teamkill_limit
local teamkill_broadcast_after = server.teamkill_broadcast_after
local teamkill_bantime = server.teamkill_bantime

local function onTeamkill(actor, victim)

    actor = server.new_player_object(actor)
    
    if not actor:vars().shown_teamkill_warning then
        actor:msg("WARNING: The server enforces a teamkill limit!")
        actor:vars().shown_teamkill_warning = true
    end
    
    local actor_teamkills = actor:teamkills()
    
	if teamkill_broadcast_after > 0 and actor_teamkills > teamkill_broadcast_after then
        server.msg(red(string.format("%s has fragged teammates %s times", white(actor:displayname()), white(actor_teamkills))))
	end
    
	if actor_teamkills > teamkill_limit then
        actor:kick(teamkill_bantime, "server", "teamkilling")
    end
end

local teamkill_event = server.event_handler("teamkill", onTeamkill)

return {unload = function()
    server.cancel_handler(teamkill_event)
end}
