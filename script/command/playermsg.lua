--[[

	A player command to send a message to a player

]]


return function(cn,tcn,text)

	if not tcn then

		return false, "#playermsg (<cn>|\"<name>\") \"<text>\""
	end

	if not text then

		return false, "text is missing"
	end

	if not server.valid_cn(tcn) then

		tcn = server.name_to_cn_list_matches(cn,tcn)

		if not tcn then

			return
		end
	end

	server.player_msg(tcn,"(" .. green("PM") .. ")  (" .. green(server.player_name(cn)) .. " (" .. magenta(cn) .. ")): " .. text)

end 
