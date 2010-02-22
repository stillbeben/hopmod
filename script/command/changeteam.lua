-- #changeteam [<cn>] [<team>]

local function other_team(cn)

	if server.player_team(cn) == "evil" then
		return "good"
	else
		return "evil"
	end

end

return function(cn, arg1, arg2)

	local tcn = nil
	local team = nil

	if not ( server.player_priv_code(cn) > 0 ) then
		tcn = cn
		team = other_team(tcn)
	else
		if not arg1 then
			tcn = cn
			team = other_team(tcn)
		elseif server.valid_cn(arg1) then
			tcn = arg1

			if not arg2 then
				team = other_team(tcn)
			else
				team = arg2
			end
		else
			tcn = cn
			team = arg1
		end
	end

	local mode = server.gamemode

	if ( mode == "insta ctf" or mode == "insta protect" or mode == "ctf" or mode == "protect" ) and not ( team == "good" or team == "evil" ) then
		return false, "Only good and evil are allowed in [insta] ctf|protect"
	end

	if not (team == server.player_team(tcn)) then
		server.changeteam(tcn,team)
	end

end
