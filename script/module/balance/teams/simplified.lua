--[[

	simplified balancer

		disallow other teams than good and evil [not optional]
		blocks teamswitchs against the balance
		moves spectator leaving players, when it is forced by balance
		moves dead players, that cry balance, while they are in the fuller team
    
]]


local using_moveblock = server.teambalance_using_moveblock
local using_player_moving_balancer_when_leaving_spec = server.teambalance_using_player_moving_balancer_when_leaving_spec
local using_text_addin = server.teambalance_using_text_addin

if server.botbalance == 0 then
	server.botbalance = 1
end

local event = {}


local function team_size(team)

	local size = 0

	for a,cn in ipairs(server.team_players(team)) do
		if not ( server.player_status_code(cn) == 5 ) then
			size = size + 1
		end
	end

	return size

end


local function other_team(team)

	if team == "evil" then
		return "good"
	else
		return "evil"
	end

end


local function fuller_team()

	if team_size("good") > team_size("evil") then
		return "good"
	else
		return "evil"
	end

end


local function team_diff()

	return (math.abs(team_size("good") - team_size("evil")))

end


local function unbalanced()

	if team_diff() > 1 then
		return true
	else
		return false
	end

end


local function is_enabled()

	if gamemodeinfo.teams and not (server.gamemode == "coop edit") and server.mastermode == 0 then -- activate conditions
		return true
	else
		return false
	end

end


if using_player_moving_balancer_when_leaving_spec == 1 then
	event.spectator = server.event_handler_object("spectator", function(cn, joined)

		if is_enabled() and joined == 0 and unbalanced() then
			local fuller = fuller_team()

			if server.player_team(cn) == fuller then
				server.changeteam(cn,other_team(fuller))

				server.player_msg(cn,"You switched the team for balance")
			end
		end

	end)
end


event.chteamrequest = server.event_handler_object("chteamrequest", function(cn, old, new)

	if is_enabled() then
		if not (new == "good" or new == "evil") then
			server.player_msg(cn,red("Only teams good and evil are allowed."))
			return (-1)
		elseif not (server.player_status_code(cn) == 5) and using_moveblock == 1 then
			if (math.abs((team_size(old) - 1) - (team_size(new) + 1))) > 1 then
				server.player_msg(cn, red(string.format("Team change disallowed: \"%s\" team has enough players.", new)))
				return (-1)
			end
		end
	end

end)


if using_text_addin == 1 then
	event.text = server.event_handler_object("text", function(cn, text)

		if is_enabled() and unbalanced() then
			local fuller = fuller_team()

			if server.player_team(cn) == fuller and server.player_status(cn) == "dead" and (string.match(text,"balance") or string.match(text,"BALANCE")) then
				server.changeteam(cn,other_team(fuller))
				server.player_msg(cn,"You switched the team for balance")
			end
		end

	end)
end


event.finishedgame = server.event_handler_object("finishedgame", function()

	if is_enabled() then
		local flag = 0

		for p in server.aplayers() do
			local pteam = p:team()

			if not ( (pteam == "good") or (pteam == "evil") ) then
				if flag == 0 then
					p:changeteam("good")

					flag = 1
				else
					p:changeteam("evil")

					flag = 0
				end
			end
		end
	end

end)


local function unload()

	event = {}

end


return {unload = unload}
