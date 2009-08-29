server.event_handler("setmastermode",function(old,new)
    if not ( new == "open" or new == "veto" ) then
	return (-1)
    end
end)
