--[[
    A player command to enable/ disable no ties module

]]


local msg = {}
msg.info = "#noties [0|off|1|on]"
msg.no_module = "no_tie module is not loaded"
msg.enabled = "no ties module enabled."
msg.disabled = "no ties module disabled."


return function(cn, option)

    if not server.no_ties
    then
	return false, msg.no_module
    end
    
    if not option
    then
	return false, msg.info
    end
    
    if (option == "0") or (option == "off")
    then
	server.no_ties()
	server.player_msg(cn, msg.disabled)
    elseif (option == "1") or (option == "on")
    then
	server.no_ties(true)
	server.player_msg(cn, msg.enabled)
    else
	return false, msg.info
    end
end
