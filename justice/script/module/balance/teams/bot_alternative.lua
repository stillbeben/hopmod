--[[

	alternative team balancing with bots
    
]]

local using_moveblock = server.teambalance_using_moveblock
local using_player_moving_balancer_when_leaving_spec = server.teambalance_using_player_moving_balancer_when_leaving_spec
local using_text_addin = server.teambalance_using_text_addin

local bot_skill_low = server.teambalance_bot_skill_low
local bot_skill_high = server.teambalance_bot_skill_high
if (bot_skill_high or 0) < bot_skill_low then
	bot_skill_high = bot_skill_low
end

if server.botbalance == 1 then
	server.botbalance = 0
end

local is_intermission = false

local function team_size(team)

	local size = 0

	for a,cn in ipairs(server.team_players(team)) do
		if not (server.player_status_code(cn) == 5) then
			size = size + 1
		end
	end

	return size
end

local function team_size_with_bots(team)

	local size = team_size(team)

	local a = server.botcount

	while a > 0 do
		local tmp = 127 + a

		if server.player_team(tmp) == team then
			size = size + 1
		end

		a = a - 1
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

local function addbots(num)

	if num == 0 then
		return
	end

	for x = 1, num do
		server.addbot(math.random(bot_skill_low,bot_skill_high))
	end

	if num == 1 then
		server.msg("The server has added a random skilled bot to balance the teams.")
	else
		server.msg(string.format("The server has added %s random skilled bots to balance the teams.", orange(num)))
	end
end

local function delbots(num)

	if num == 0 or server.botcount == 0 then
		return
	end

	for x = 1, num do
		server.delbot()
		server.log("Server removed bot")
	end
end

local function remove_all_bots()
	delbots(server.botcount)
end

local function balance()

	if unbalanced() then
		local fuller = fuller_team()
		local other = other_team(fuller)

		local diff = (team_size(fuller) - 1) - team_size_with_bots(other)

		if not (diff == 0) then
			local change_function = addbots

			if diff < 0 then
				change_function = delbots
			end

			change_function(math.abs(diff))
		end
	else
		remove_all_bots()
	end
end

local function check_balance(option)

	if not option then
		option = 500
	end

	server.sleep(option,function()
		balance()
	end)
end

local function is_enabled()

	if gamemodeinfo.teams and not (server.gamemode == "coop edit") and server.mastermode == 0 and is_intermission == false then -- activate conditions
		return true
	else
		return false
	end
end

server.event_handler("spectator", function(cn, joined)

	if is_enabled() then
		if joined == 1 then
			if (server.playercount - server.speccount) == 0 then
				remove_all_bots()
			else
				check_balance(5000)
			end
		elseif (server.playercount - server.speccount) == 1 then
			local bots = server.botcount

			if bots == 0 then
				addbots(1)
			elseif bots > 1 then
				delbots(bots - 1)
			end
		elseif unbalanced() then
			if using_player_moving_balancer_when_leaving_spec == 1 then
				local fuller = fuller_team()

				if server.player_team(cn) == fuller then
					server.changeteam(cn,other_team(fuller))

					server.player_msg(cn,"You switched the team for balance")
				end
			end

			check_balance()
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

server.event_handler("reteam", function(cn, old, new)

	if is_enabled() then
		if (server.playercount - server.speccount) == 1 then
			local bots = server.botcount

			if bots == 0 then
				addbots(1)
			elseif bots > 1 then
				delbots(bots - 1)
			end
		else
			check_balance()
		end
	end

end)

if using_text_addin == 1 then
	event.text = server.event_handler("text", function(cn, text)

		if is_enabled() and unbalanced() then
			local fuller = fuller_team()

			if server.player_team(cn) == fuller and server.player_status(cn) == "dead" and (string.match(text,"balance") or string.match(text,"BALANCE")) then
				server.changeteam(cn,other_team(fuller))
				server.player_msg(cn,"You switched the team for balance")

				check_balance()
			end
		end

	end)
end

server.event_handler("finishedgame", function()

	is_intermission = false

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

	if is_enabled() then
		remove_all_bots()
	end

	is_intermission = true
end)

server.event_handler("mapchange", function(map, mode)
	if is_enabled() then
		if (server.playercount - server.speccount) == 1 then
			local bots = server.botcount

			if bots == 0 then
				addbots(1)
			elseif bots > 1 then
				delbots(bots - 1)
			end
		else
			check_balance(10000)
		end
	end
end)

server.event_handler("connect", function(cn)
	if is_enabled() then
		if (server.playercount - server.speccount) == 1 then
			local bots = server.botcount

			if bots == 0 then
				addbots(1)
			elseif bots > 1 then
				delbots(bots - 1)
			end
		else
			check_balance()
		end
	end
end)

server.event_handler("disconnect", function(cn, reason)

	if is_enabled() then
		if (server.playercount - server.speccount) == 1 then
			local bots = server.botcount

			if bots == 0 then
				addbots(1)
			elseif bots > 1 then
				delbots(bots - 1)
			end
		else
			check_balance(10000)
		end
	end

end)

server.event_handler("setmastermode", function(cn, current, new)

	if is_enabled() and not new == "open" then

		server.sleep(750, function()	-- prevent conflicts with disallow_mastermodes_for_admins

			if not is_enabled() then
				server.player_msg(cn, "Auto Team Balancing has been disabled. It will be re-enabled once the bots have been removed and/or the mastermode has been set to OPEN(0).")

				remove_all_bots()
			end

		end)

	end
end)

local function unload()
	remove_all_bots()
	is_intermission = false
end

check_balance(1000) -- in case this module was loaded from #reload

return {unload = unload}
