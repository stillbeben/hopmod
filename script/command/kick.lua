-- #kick [all] <cn> ["<reason>"]

return function(cn,arg1,arg2,arg3)

	local all = 0
	local cn = nil
	local reason = ""

	if not arg1 then
		server.player_msg(kick_cn,red("#kick <cn> [\"<reason>\"]"))
		return
	elseif arg1 == "all" then
		all = 1
		if not arg2 then
			server.player_msg(kick_cn,red("cn is missing"))
			return
		elseif server.valid_cn(arg2) then
			cn = arg2
		else
			server.player_msg(kick_cn,red("cn is not valid"))
			return
		end
		if arg3 then
			reason = arg3
		end
	elseif server.valid_cn(arg1) then
		cn = arg1
		if arg2 then
			reason = arg2
		end
	else
		server.player_msg(kick_cn,red("cn is not valid"))
		return
	end

	if all == 1 then
		server.kick_bannedip_group = 1
	end
	server.kick(cn,1,server.player_name(kick_cn),reason)
	if all == 1 then
		server.kick_bannedip_group = 0
	end

end
