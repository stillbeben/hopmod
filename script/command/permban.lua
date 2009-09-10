local pb_file = io.open(server.banlist_file, "a+")

local function permban(cn_ban, ip_ban)
	pb_file:write("permban " .. ip_ban .. "\n")
	server.kick(cn_ban, -1, "", red("permbanned"))
	pb_file:flush()
end

return function(cn, cn_ban)
	permban(cn_ban, server.player_ip(cn_ban))
end