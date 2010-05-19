local disallow_mastermode_open = server.disallow_mastermode_open_for_admins
local disallow_mastermode_veto = server.disallow_mastermode_veto_for_admins
local disallow_mastermode_locked = server.disallow_mastermode_locked_for_admins
local disallow_mastermode_private = server.disallow_mastermode_private_for_admins

server.event_handler("setmastermode", function(cn, old, new)

	if   (disallow_mastermode_open == 1 and new == "open") or
	     (disallow_mastermode_veto == 1 and new == "veto") or 
	     (disallow_mastermode_locked == 1 and new == "locked") or 
	     (disallow_mastermode_private == 1 and new == "private") then
		    server.player_msg(cn, red("Mastermode " .. new .. " is disabled."))
		    return -1
	end
end)

