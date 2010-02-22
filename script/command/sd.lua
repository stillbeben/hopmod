-- #sd
return function(cn)

	if not server.cmd_sd then
		return false, "suddendeath module is not loaded"
	end

	server.cmd_sd()

end
