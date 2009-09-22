suddendeath = {}
suddendeath.is_active = nil


function suddendeath.active_events_disable()

	suddendeath.is_active = 0

	if suddendeath.event_frag then
		server.cancel_handler(suddendeath.event_frag)
		suddendeath.event_frag = nil
	end

	if suddendeath.event_suicide then
		server.cancel_handler(suddendeath.event_suicide)
		suddendeath.event_suicide = nil
	end

	suddendeath.no_event = nil

	if suddendeath.event_scoreflag then
		server.cancel_handler(suddendeath.event_scoreflag)
		suddendeath.event_scoreflag = nil
	end

end


function suddendeath.no_event()

	if suddendeath.is_active == 1 then
		local break_first = 0
		local draw = 0

		for index,name in ipairs(server.teams()) do
			local tscore = server.team_score(name)

			for i,n in ipairs(server.teams()) do
				if not (name == n) then
					if tscore == server.team_score(n) then
						draw = 1
						break_first = 1
						break
					end
				end
			end

			if break_first == 1 then
				break
			end
		end

		if draw == 0 then
			suddendeath.is_active = 0
			server.changetime(10)
			return true
		end
	end

	if suddendeath.is_active == 0 then
		return true
	end

	return false

end


function suddendeath.active_events_enable(option)

	if not option then
		return (-1)
	end

	if (option == 0) or (option == 3) then
		if not suddendeath.event_frag then
			server.msg(red("Sudden Death. Next Frag Wins!"))

			suddendeath.event_frag = server.event_handler("frag",function(tcn,acn)

				if suddendeath.is_active == 1 then
					suddendeath.is_active = 0
					server.changetime(10)
				end

			end)

		end

		if not suddendeath.event_suicide then

			suddendeath.event_suicide = server.event_handler("suicide",function(cn)

				if suddendeath.is_active == 1 then
					suddendeath.is_active = 0
					server.changetime(10)
				end

			end)

		end
	elseif option == 1 then
		if not suddendeath.no_event then
			server.msg(red("Sudden Death. Next Score Wins!"))
			suddendeath.no_event = 1

			server.interval(1000,function()

				if suddendeath.no_event() then
					return (-1)
				end

			end)

		end
	elseif option == 2 then
		if not suddendeath.event_scoreflag then
			server.msg(red("Sudden Death. Next Score Wins!"))

			suddendeath.event_scoreflag = server.event_handler("scoreflag",function(cn)

				if suddendeath.is_active == 1 then
					suddendeath.is_active = 0
					server.changetime(10)
				end

			end)

		end
	elseif option == 4 then
		if not suddendeath.event_scoreflag then
			server.msg(red("Sudden Death. Next Score Wins!"))

			suddendeath.event_scoreflag = server.event_handler("scoreflag",function(cn)

				if suddendeath.is_active == 1 then
					suddendeath.is_active = 0
					server.changetime(10)
				end

			end)

		end

		if not suddendeath.no_event then
			suddendeath.no_event = 1

			server.interval(1000,function()

				if suddendeath.no_event() then
					return (-1)
				end

			end)

		end
	end

	suddendeath.is_active = 1

end


function suddendeath.disable()

	if suddendeath.event_timeupdate then
		server.cancel_handler(suddendeath.event_timeupdate)
		suddendeath.event_timeupdate = nil
	end

	if suddendeath.event_finishedgame then
		server.cancel_handler(suddendeath.event_finishedgame)
		suddendeath.event_finishedgame = nil
	end

	suddendeath.active_events_disable()

end


function suddendeath.enable()

	if not suddendeath.event_timeupdate then

		suddendeath.checked_mode = 0

		suddendeath.event_timeupdate = server.event_handler("timeupdate",function(mins)

			if mins == 1 then

				server.sleep(58000,function()

					local gmode = tostring(server.gamemode)
					local break_first = 0
					local draw = 1

					if suddendeath.checked_mode == 0 then
						if gmode == "ffa" or gmode == "tactics" or gmode == "instagib" or gmode == "efficiency" then
							for player in server.gplayers() do
								local pfrags = player:frags()

								for p in server.gplayers() do
									if not (player.cn == p.cn) then
										if not (pfrags == p:frags()) then
											draw = 0
											break_first = 1
											break
										end
									end
								end

								if break_first == 1 then
									break
								end
							end

							if draw == 1 then
								suddendeath.active_events_enable(0)
							end
						else
							for index,name in ipairs(server.teams()) do
								local tscore = server.team_score(name)

								for i,n in ipairs(server.teams()) do
									if not (name == n) then
										if not (tscore == server.team_score(n)) then
											draw = 0
											break_first = 1
											break
										end
									end
								end

								if break_first == 1 then
									break
								end
							end

							if draw == 1 then
								if gmode == "regen capture" or gmode == "capture" then
									suddendeath.active_events_enable(1)
								elseif gmode == "insta ctf" or gmode == "ctf" then
									suddendeath.active_events_enable(2)
								elseif gmode == "insta protect" or gmode == "protect" then
									suddendeath.active_events_enable(4)
								elseif gmode == "teamplay" or gmode == "tactics team" or gmode == "instagib team" or gmode == "efficiency team" then
									suddendeath.active_events_enable(3)
								end
							end
						end

						suddendeath.checked_mode = 1
					end

				end)

			end

			if suddendeath.is_active == 1 then
                return 1
			else
				return mins
			end

		end)

	end

	if not suddendeath.event_finishedgame then

		suddendeath.event_finishedgame = server.event_handler("finishedgame",function()

--			suddendeath.active_events_disable()
			suddendeath.disable()

		end)

	end

end


return {unload = suddendeath.disable()}
