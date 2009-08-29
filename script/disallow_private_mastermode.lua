server.event_handler("setmastermode",function(old,new)
    if new == "private" then
	return (-1)
    end
end)
