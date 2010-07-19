--[[

	A player command to kick a player

]]


return function(cn,arg1,arg2,arg3)

	local all = 0
	local lcn = nil
	local reason = ""

	if not arg1 then

		return false, "#kick <cn> [\"<reason>\"]"

	elseif arg1 == "all" then

		all = 1

		if not arg2 then

			return false, "CN is missing"

		elseif server.valid_cn(arg2) then

			lcn = arg2
		else

			return false, "CN is not valid"
		end
		if arg3 then

			reason = arg3
		end

	elseif server.valid_cn(arg1) then

		lcn = arg1

		if arg2 then

			reason = arg2
		end

	else

		return false, "CN is not valid"
	end

	if lcn == cn then

		return false, "Don't kick yourself"
	else

		if all == 1 then

			server.kick_bannedip_group = true
		end

		server.kick(lcn,1,server.player_name(cn),reason)

		if all == 1 then

			server.kick_bannedip_group = false
		end
	end

end
