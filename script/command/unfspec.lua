--[[

	A player command to disable a block of "cd/modmap"

]]


local usage = "#unfspec <cn>|\"<name>\""


return function(cn,tcn)

	if not tcn then

		return false, usage
	end

	if not server.valid_cn(tcn) then

		tcn = server.name_to_cn_list_matches(cn,tcn)

		if not tcn then

			return
		end
	end

	tcn = tonumber(tcn)

	server.player_vars(tcn).modmap = false
	server.unspec(tcn)

end
