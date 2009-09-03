-- #group [all] <tag> [<team>]

local cmd_group = {}

function cmd_group.playercmd(cn,arg1,arg2,arg3)
    if not server.gamemodeinfo.teams and server.reassignteams == 1 then
        return
    end
    if not arg1 then
        server.player_msg(cn,red("#group [all] <tag> [<team>]"))
        return
    end
    local tag = nil
    local team = nil
    if arg1 == "all" then
        if not arg2 then
            server.player_msg(cn,red("missing tag argument"))
            return
        end
        tag = arg2
        if arg3 then
            team = arg3
        else
    	team = tag
        end
        for j,cn in ipairs(server.spectators()) do
            if string.find(server.player_name(cn),tag) then
                server.unspec(cn)
            end
        end
    else
        tag = arg1
        if arg2 then
            team = arg2
        else
    	    team = tag
        end
    end
    for p in server.gplayers() do
        if string.find(p:name(),tag) then
            server.changeteam(p.cn,team)
        end
    end
end


if server.group_command_master == 1 then
    function server.playercmd_group(cn,arg1,arg2,arg3)
	return mastercmd(function()
    	    cmd_group.playercmd(cn,arg1,arg2,arg3)
	end,cn)
    end
else
    function server.playercmd_group(cn,arg1,arg2,arg3)
	return admincmd(function()
    	    cmd_group.playercmd(cn,arg1,arg2,arg3)
	end,cn)
    end
end
