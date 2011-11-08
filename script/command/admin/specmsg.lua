--[[

	A player command to send a message to all spectators

]]

local usage = "#specmsg <text>"


return function(cn, ...)

	local text = table.concat(arg, " ")
    
    if not text then
        return false, usage
    end

	for client in server.gspectators() do
		server.player_msg(p.cn,"(" .. green("spec-message") .. ")  (" .. green(server.player_name(cn)) .. " (" .. magenta(cn) .. ")): " .. text)
	end

end
