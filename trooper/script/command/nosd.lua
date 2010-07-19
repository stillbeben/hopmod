-- #nosd
return function(cn)

	if not server.cmd_nosd then
		return false, "suddendeath module is not loaded"
	end

	server.cmd_nosd()

end
