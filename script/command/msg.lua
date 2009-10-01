--[[

	A player command to broadcast a message

]]


return function(cn,text)

	if not text then

		return false, "#msg \"<text>\""
	end

	server.msg("(" .. green("Info") .. ")  " .. text)

end
