
local usage = "#unban \"<ip>\""

return function(cn, ip)

	if not ip then
		return false, usage
	end
    
    local res = check_ip(ip)
    
    if #res == 1 then
        return false, string.format("Invalid IP (%s )", res[1])
    end
    
	server.unban(ip)
    
	server.player_msg(cn, "done, check with #banlist")

end
