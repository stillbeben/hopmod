function server.playercmd_changeteam(changeteam_cn,arg1,arg2)
    local function other_team(other_team_cn)
	if server.player_team(other_team_cn) == "evil" then
    	    return "good"
    	else
    	    return "evil"
    	end
    end
    
    local cn = nil
    local team = nil
    if not ( server.player_priv_code(changeteam_cn) > 0 ) then
    	cn = changeteam_cn
    	team = other_team(cn)
    else
	if not arg1 then
	    cn = changeteam_cn
	    team = other_team(cn)
	elseif server.valid_cn(arg1) then
	    cn = arg1
	    if not arg2 then
		team = other_team(cn)
	    else
		local gmode = tostring(server.gamemode)
		if ( gmode == "insta ctf" or gmode == "insta protect" or gmode == "ctf" or gmode == "protect" ) and not ( arg2 == "good" or arg2 == "evil" ) then
		    server.player_msg(changeteam_cn,red("only good and evil are allowed in [insta] ctf|protect"))
		    return
		else
		    team = arg2
		end
	    end
	else
	    cn = changeteam_cn
	    team = arg1
	end
    end
    if team ~= server.player_team(cn) then
	server.changeteam(cn,team)
    end
end 

function server.playercmd_cteam(cteam_cn,arg1,arg2)
    server.playercmd_changeteam(cteam_cn,arg1,arg2)
end
