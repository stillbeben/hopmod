-- #givemaster <cn>

function server.playercmd_givemaster(cn, target)
    return mastercmd(function ()
        if server.player_id(target) == -1 then
            server.player_msg(cn, red("Player not found."))
	    return
        end
        server.unsetmaster()
        server.player_msg(target, server.player_name(cn) .. " has passed master privilege to you.")
        server.setmaster(target)
    end, cn)
end
