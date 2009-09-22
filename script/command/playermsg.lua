-- #playermsg <cn> "<text>"

return function(cn,tcn,text)

	if not tcn then
		server.player_msg(cn,red("#playermsg <cn> \"<text>\""))
	elseif not text then
		server.player_msg(cn,red("text is missing"))
	elseif not server.valid_cn(tcn) then
		server.player_msg(privmsg_cn,red("cn is not valid"))
	else
		server.player_msg(tcn,"(" .. green("PM") .. ")  (" .. green(server.player_name(cn)) .. "(" .. magenta(cn) .. ")): " .. text)
	end

end 
