server.event_handler("shot",function(cn,gun,hits)
    if cn < 128 then
	local gametime = server.gamemillis
  	local reload_limit = nil
  	if server.player_vars(cn).cdr_last_weapon and not (gun == server.player_vars(cn).cdr_last_weapon) then
    	    server.player_vars(cn).cdr_last_shot = nil
  	end
  	if gun == 2 then
    	    reload_limit = 80
	elseif gun == 0 then
	    reload_limit = 230
	elseif gun == 6 then
	    reload_limit = 350
	elseif gun == 5 then
	    reload_limit = 480
	elseif gun == 3 then
	    reload_limit = 780
	elseif gun == 1 then
	    reload_limit = 1380
	elseif gun == 4 then
	    reload_limit = 1460
	end
  	if server.player_vars(cn).cdr_last_shot and ((gametime - server.player_vars(cn).cdr_last_shot) < reload_limit) then
    	    server.log("WARNING: " .. server.player_name(cn) .. "(" .. cn .. ")'s weapon reload time is too low.  [pj: " .. server.player_lag(cn) .. " | ping: " .. server.player_ping(cn) .. " | weapon: " .. gun .. " | ip: " .. server.player_ip(cn) .. "]")
  	end
  	server.player_vars(cn).cdr_last_shot = gametime
  	server.player_vars(cn).cdr_last_weapon = gun
    end
end)
