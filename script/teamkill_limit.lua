local teamkill_limit = server.teamkill_limit
local teamkill_broadcast_after = server.teamkill_broadcast_after
local teamkill_bantime = server.teamkill_bantime

local event = {}


event.teamkill = server.event_handler("teamkill", function(actor, victim)

	actor = server.new_player_object(actor)

	if not actor:vars().shown_teamkill_warning then

		actor:msg("WARNING: The server enforces a teamkill limit!")
		actor:vars().shown_teamkill_warning = true

	end

	local actor_teamkills = actor:teamkills()

	if teamkill_broadcast_after > 0 and actor_teamkills > teamkill_broadcast_after then

		server.msg(red(string.format("%s has fragged teammates %s times", white(actor:displayname()), white(actor_teamkills))))

	end

	if actor_teamkills + (actor:pvars().tklimiter_last_tks or 0) > teamkill_limit then

		actor:kick(teamkill_bantime, "server", "teamkilling")

	end

end)


if server.teamkill_dynamic_limit == 1 then

	local teamkill_limit_1 = round(teamkill_limit / 4)
	local teamkill_limit_2 = round(teamkill_limit / 2)
	local teamkill_limit_3 = teamkill_limit_1 * 3


	local function check_tks(cn)

		local current_tks = server.player_teamkills(cn) + round((server.player_pvars(cn).tklimiter_last_tks or 0) / 3)

		if current_tks >= teamkill_limit_3 then
			server.player_pvars(cn).tklimiter_last_tks = teamkill_limit_2
		elseif current_tks >= teamkill_limit_2 then
			server.player_pvars(cn).tklimiter_last_tks = teamkill_limit_1
		elseif current_tks >= teamkill_limit_1 then
			server.player_pvars(cn).tklimiter_last_tks = 1                      -- fix.me?!
		else
			server.player_pvars(cn).tklimiter_last_tks = 0
		end

	end


	event.finishedgame = server.event_handler_object("finishedgame",function()

		for p in server.aplayers() do
			check_tks(p.cn)
		end

	end)

end


local function unload()

	event = {}

	for p in server.aplayers() do
		p:pvars().tklimiter_last_tks = nil
	end

end


return {unload = unload}
