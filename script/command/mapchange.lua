-- #forcemap [<mode>] [<map>] [<time>]
-- #changemap [<mode>] [<map>] [<time>]


local allowed_modes = table_unique(server.parse_list(server.allowed_gamemodes))


local function forcemap(cn, arg1, arg2, arg3)

	local map = "random"
	local mode = server.gamemode
	local time = -1

	local function parse_map_and_time(arg1, arg2)

		if arg1 then
			local tmp = tonumber(arg1)

			if tmp and tmp > 0 and tmp <= 13669 then
				time = tmp
			else
				map = arg1

				if arg2 then
					local tmp = tonumber(arg2)

					if tmp and tmp > 0 and tmp <= 13669 then
						time = tmp
					else
						return false
					end
				end
			end
		end

		return true

	end

	if arg1 then
		mode = server.parse_mode(arg1)

		if not mode then
			if not parse_map_and_time(arg1,arg2) then
				return false, "time is not valid"
			else
				mode = server.gamemode
			end
		else
			if not parse_map_and_time(arg2,arg3) then
				return false, "time is not valid"
			end
		end
	end

	if map == "random" then
		map = server.random_map(mode)
	end

	server.changemap(map, mode, time)

end


local function changemap(cn, arg1, arg2, arg3)

	local mode = server.gamemode
	local map = "random"

	if arg1 then
		mode = server.parse_mode(arg1)

		if mode then
			local flag = false

			for i,gmode in ipairs(allowed_modes) do
				if gmode == mode then
					flag = true
				end
			end

			if flag == false then
				return false, "mode is not allowed"
			end

			if arg2 then
				local tmp = tonumber(arg2)
				if not ( tmp and tmp > 0 and tmp <= 13669 ) then
					map = arg2
				end
			end
		else
			mode = server.gamemode

			local tmp = tonumber(arg1)

			if not ( tmp and tmp > 0 and tmp <= 13669 ) then
				map = arg1
			end
		end

		if not (map == "random") then
			if not server.is_known_map(map) then
				return false, "map is not known"
			end

			if not server.is_known_map(map, mode) then
				return false, "map is not allowed"
			end
		end


	end

	forcemap(cn, arg1, arg2, arg3)

end


return {
	changemap_cmd = changemap,
	forcemap_cmd = forcemap
}
