local tk_limit = server.teamkill_limit_flag_carrier
local ban_time = server.teamkill_bantime
local is_active = false

server.event_handler("dropflag", function(cn)

	server.player_vars(cn).has_flag = nil
end)

server.event_handler("takeflag", function(cn)

	server.player_vars(cn).has_flag = true
end)

server.event_handler("teamkill", function(acn, tcn)

	if is_active == true and server.player_vars(tcn).has_flag then
		local tks_actor = (server.player_vars(acn).tks_flag_carrier or 0) + 1

		if tks_actor > tk_limit then
			server.kick(acn,ban_time,"server","teamkilling")
		else
			server.player_vars(acn).tks_flag_carrier = tks_actor
		end
	end
end)

server.event_handler("mapchange", function(map, mode)

	if mode == "insta ctf" or mode == "ctf" or mode == "insta protect" or mode == "protect" then
		is_active = true
	else
		is_active = false
	end

	for p in server.aplayers() do
		p:vars().has_flag = nil
	end

end)

local gmode = server.gamemode

if gmode == "insta ctf" or gmode == "ctf" or gmode == "insta protect" or gmode == "protect" then
	is_active = true
end

local function unload()

	is_active = false
	
    for p in server.aplayers() do
        p:vars().has_flag = nil
    end
end

return {unload = unload}
