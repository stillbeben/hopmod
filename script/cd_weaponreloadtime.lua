server.event_handler("shot",function(cn,gun,hits)
    if cn < 128 then
	local gametime = server.gamemillis
  	local reload_limit = nil
  	if server.player_vars(cn).cdr_last_weapon and not (gun == server.player_vars(cn).cdr_last_weapon) then
    	    server.player_vars(cn).cdr_last_shot = nil
  	end
  	if gun == 2 then
    	    reload_limit = 70
	elseif gun == 0 then
	    reload_limit = 220
	elseif gun == 6 then
	    reload_limit = 340
	elseif gun == 5 then
	    reload_limit = 470
	elseif gun == 3 then
	    reload_limit = 770
	elseif gun == 1 then
	    reload_limit = 1370
	elseif gun == 4 then
	    reload_limit = 1450
	end
  	if server.player_vars(cn).cdr_last_shot and ((gametime - server.player_vars(cn).cdr_last_shot) < reload_limit) then
    	    server.log("WARNING: " .. server.player_name(cn) .. "(" .. cn .. ")'s weapon reload time is too low. [weapon: " .. gun .. " | ip: " .. server.player_ip(cn) .. "]")
  	end
  	server.player_vars(cn).cdr_last_shot = gametime
  	server.player_vars(cn).cdr_last_weapon = gun
    end
end)
