-- example file for "perma-bans"
-- uncomment functions and edit!

-- local function check_bans_mutes(cn)
--     if
--         (string.match(server.player_name(cn),"UNWANTED.PATTERN"))	-- case sensitive check if pattern is in name
--      or
--         (string.match(string.lower(server.player_name(cn)),"unwanted.pattern"))	-- case insensitve check if pattern is in name
--      or
--     	(string.match(server.player_name(cn),"UNWANTED.PATTERN") and string.match(server.player_ip(cn,"^84.47.*")))	-- name and ip check
--      or
-- 	(server.player_name(cn) == "unwanted.name")	-- check if name is exactly unwanted.name
--      or
--         ...
--     then
--         server.kick(cn,"1","server","")
--     end
--     
--     if
-- 	...	-- same like above, but mute instead kick
--     then
-- 	server.mute(cn)
--     end
-- end

-- server.event_handler("connect",check_bans_mutes)

-- server.event_handler("rename",function(cn,old,new)
--     check_bans_mutes(cn)
-- end)
