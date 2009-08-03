server.event_handler("shot",function(cn,gun,hits)
    local reload_limit = nil
    if server.player_vars(cn).cdr_last_weapon and not (gun == server.player_vars(cn).cdr_last_weapon) then
	server.player_vars(cn).cdr_last_shot = nil
    end
    if gun == 0 then
	reload_limit = 245
    elseif gun == 1 then
	reload_limit = 1395
    elseif gun == 2 then
	reload_limit = 95
    elseif gun == 3 then
	reload_limit = 795
    elseif gun == 4 then
	reload_limit = 1495
    elseif gun == 5 then
	reload_limit = 495
    elseif gun == 6 then
	reload_limit = 495
    end
    if server.player_vars(cn).cdr_last_shot and ((server.gamemillis - server.player_vars(cn).cdr_last_shot) < reload_limit) then
	server.log(server.player_name(cn) .. "'s weapon reload time is too low. [cn: " .. cn .. " | weapon: " .. gun .. "]")
    end
    server.player_vars(cn).cdr_last_shot = server.gamemillis
    server.player_vars(cn).cdr_last_weapon = gun
end)
