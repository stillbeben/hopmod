-- #getcn "<playername>"

function server.playercmd_getcn(getcn_cn,pname)
    if not pname then
	server.player_msg(getcn_cn,red("#getcn \"<playername>\""))
    else
	local tmp = server.find_cn(getcn_cn,pname)
	if not tmp then
	    return
	else
	    server.player_msg(getcn_cn,green(pname) .. "'s cn is " .. green(tmp))
	end
    end
end
