--[[

	player moving team balancing

]]

local using_moveblock = server.teambalance_using_moveblock
local using_player_moving_balancer_when_leaving_spec = server.teambalance_using_player_moving_balancer_when_leaving_spec
local using_text_addin = server.teambalance_using_text_addin

local is_intermission = false
local search_dead_player = false

local function team_size(team)

	size = 0

	for a,cn in ipairs(server.team_players(team)) do
		if not (server.player_status_code(cn) == 5) then
			size = size + 1
		end

		return size
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

local function move_player(cn, team)

	if server.player_team(cn) == team then
		server.changeteam(cn,other_team(team))
		server.player_msg(cn,string.format(orange("You switched the team for balance.")))

		search_dead_player = false

		check_balance(5000)
	end
end

server.event_handler("frag", function(tcn, acn)

	if search_dead_player == true then
		move_player(tcn, fuller_team())
	end
end)

server.event_handler("suicide", function(cn)

	if search_dead_player == true then
		move_player(cn, fuller_team())
	end
end)

local function check_already_dead_players(team)

	for a, cn in ipairs(server.team_players(team)) do
		if server.player_status(cn) == "dead" then
			move_player(cn, team)
			return
		end
	end

	search_dead_player = true
end

local function balance()
		   
	if unbalanced() then
		if search_dead_player == false then
			check_already_dead_player(fuller_team())
		end
	elseif search_dead_player == true then
		search_dead_player = false
	end
end

local function check_balance(option)
		   
	if not option then
--		option = 15000
		option = 1000
	end

	server.sleep(option,function()
		balance()
	end)
end

local function is_enabled()

	if gamemodeinfo.teams and not (server.gamemode == "coop edit") and server.mastermode == 0 and is_intermission == false then -- activate conditions
		return true
	else
		search_dead_player = false
		return false
	end
end

server.event_handler("spectator", function(cn, joined)

	if is_enabled() then
		if joined == 1 then
			check_balance()
		elseif unbalanced() then
			if using_player_moving_balancer_when_leaving_spec == 1 then
				move_player(cn, fuller_team())
			else
				check_balance(5000)
			end
		end
	end

end)

server.event_handler("chteamrequest", function(cn, old, new)

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

event.reteam = server.event_handler("reteam", function(cn, old, new)

	if is_enabled() then
		check_balance()
	end
end)

if using_text_addin == 1 then
	server.event_handler("text", function(cn, text)

		if is_enabled() and unbalanced() then
			local fuller = fuller_team()

			if server.player_team(cn) == fuller and server.player_status(cn) == "dead" and (string.match(text,"balance") or string.match(text,"BALANCE")) then
				move_player(cn, fuller)

				check_balance()
			end
		end

	end)
end

server.event_handler("finishedgame", function()

	is_intermission = false
	search_dead_player = false

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

server.event_handler("intermission", function()
	is_intermission = true
end)

server.event_handler("mapchange", function(map, mode)

	if is_enabled() then
		check_balance()
	end
end)

server.event_handler("disconnect", function(cn, reason)

	if is_enabled() then
		check_balance()
	end
end)

local function unload()

	is_intermission = false
	search_dead_player = false
end

check_balance(10000) -- in case this module was loaded from #reload

return {unload = unload}
