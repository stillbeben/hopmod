--[[

	A player command to raise player's privilege to master

]]

local usage = "#givemaster <cn>"

return function(cn, target)

	if not target then

		return false, usage
	end

	if not server.valid_cn(target) then
		return false, "CN is not valid"
	end

	server.unsetpriv(cn)
	server.player_msg(target, server.player_displayname(cn) .. " has passed master privilege to you.")
        if not (server.player_priv_code(target) >= server.PRIV_MASTER) then
            server.setmaster(target)
        end

end
