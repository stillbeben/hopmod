-- #group [all] <tag> [<team>]

return function(cn,arg1,arg2,arg3)

	if not server.gamemodeinfo.teams and server.reassignteams == 1 then
		return
	end

	if not arg1 then
		server.player_msg(cn,red("#group [all] <tag> [<team>]"))
		return
	end

	server.group_players(arg1,arg2,arg3)

end
