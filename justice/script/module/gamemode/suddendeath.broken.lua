local no_event_active = false
local frag_event_active = false
local suicide_event_active = false
local scoreflag_event_active = false
local resetflag_event_active = false
local sudden_death_active = false
local is_unload = false

server.suddendeath_enabled = false
server.suddendeath_loaded = true

server.sleep(1000, function()	-- no event

	if is_unload == true then
		return -1
	end

	if no_event_active == true then
		for index, name in ipairs(server.teams()) do
			local tscore = server.team_score(name)

			for i, n in ipairs(server.teams()) do
				if not (name == n) and tscore == server.team_score(n) then
					server.changetime(1)
					sudden_death_active = false
					no_event_active = false
					break
				end
			end

			if no_event_active == false then
				break
			end
		end
	end

end)

server.event_handler("frag", function(tcn, acn)

	if frag_event_active == true then
		server.changetime(1)
		sudden_death_active = false
		frag_event_active = false

	end

end)

server.event_handler("suicide", function(cn)

	if suicide_event_active == true then
		server.changetime(1)
		sudden_death_active = false
		suicide_event_active = false
	end

end)

server.event_handler("scoreflag", function(cn)

	if scoreflag_event_active == true then
		server.changetime(1)
		sudden_death_active = false
		scoreflag_event_active = false
	end

end)

server.event_handler("resetflag", function()

	if resetflag_event_active == true then
		server.changetime(1)
		sudden_death_active = false
		resetflag_event_active = false
	end

end)

server.event_handler("timeupdate", function(mins)

	if mins == 1 and sudden_death_active == false and server.suddendeath_enabled == true then

		server.sleep(58000, function()

			if is_unload == true or sudden_death_active == true then
				return
			end

			local gmode = tostring(server.gamemode)

			if gmode == "ffa" or gmode == "tactics" or gmode == "instagib" or gmode == "efficiency" then
				for player in server.gplayers() do
					local pfrags = player:frags()

					for p in server.gplayers() do
						if not (player.cn == p.cn) and pfrags == p:frags() then
							frag_event_active = true
							suicide_event_active = true

							sudden_death_active = true
							server.msg(orange("Sudden Death..."))
							return
						end
					end
				end
			else
				for index, name in ipairs(server.teams()) do
					local tscore = server.team_score(name)

					for i, n in ipairs(server.teams()) do
						if not (name == n) and tscore == server.team_score(n) then
							if gmode == "regen capture" or gmode == "capture" then
								no_event_active = true
							elseif gmode == "insta ctf" or gmode == "ctf" then
								scoreflag_event_active = true
							elseif gmode == "insta protect" or gmode == "protect" then
								scoreflag_event_active = true
								resetflag_event_active = true
								no_event_active = true
							elseif gmode == "teamplay" or gmode == "tactics team" or gmode == "instagib team" or gmode == "efficiency team" then
								frag_event_active = true
								suicide_event_active = true
							end

							sudden_death_active = true
							server.msg(orange("Sudden Death..."))
							return
						end
					end
				end
			end

			sudden_death_active = false
			return

		end)

	end

	if sudden_death_active == true and server.suddendeath_enabled == true then
		return 1
	else
		return mins
	end

end)

server.event_handler("finishedgame", function()

	no_event_active = false
	frag_event_active = false
	suicide_event_active = false
	scoreflag_event_active = false
	resetflag_event_active = false

	sudden_death_active = false

end)

-- A player command to enable suddendeath mode for the current map
function server.cmd_sd(cn)

	server.msg(orange("--[ Sudden Death Mode Enabled. There will be no ties"))

	server.suddendeath_enabled = true

	no_event_active = false
	frag_event_active = false
	suicide_event_active = false
	scoreflag_event_active = false
	resetflag_event_active = false

	sudden_death_active = false

end

-- A player command to disable suddendeath mode for the current map
function server.cmd_nosd(cn)

	server.msg(orange("--[ Sudden Death Mode Disabled. There may be ties"))

	server.suddendeath_enabled = false

	no_event_active = false
	frag_event_active = false
	suicide_event_active = false
	scoreflag_event_active = false
	resetflag_event_active = false

	sudden_death_active = false

end

local function unload()
	is_unload = true
	server.suddendeath_loaded = nil
end

return {unload = unload}
