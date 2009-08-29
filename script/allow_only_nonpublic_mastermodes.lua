server.event_handler("setmastermode",function(old,new)
    if not ( new == "locked" or new == "private" ) then
	return (-1)
    end
end)
