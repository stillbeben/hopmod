-- (c) 2010 by |DM|Thomas & |noVI:pure_ascii

hide_and_seek = false
local hah_active_player = -1
local hah_seek_team = ""
local hah_next_player = -1
local hah_actor_frags = 0
local MAP_LOADED_ALL = false
local PLAYER_ACTIVE = { }
local hah_active_player_can_spawn = false
local GAME_ACTIVE = false
local caught_players = { }
local server_change_team_request = { } 
local can_vote = true
local has_time = 12 -- 12 minutes

local function reset_on_mapchange()
    hah_actor_frags = 0
    MAP_LOADED_ALL = false
    PLAYER_ACTIVE = { }
    hah_active_player_can_spawn = false
    GAME_ACTIVE = false
    PLAYER_SWITCHED = false
    caught_players = { }
    server_change_team_request = { } 
    can_vote = false
    server.changetime(1000*60*has_time) 
end

local function setteam(cn)
    local jointeam = "hide"
    
    if caught_players[cn] ~= nil or hah_active_player == cn then
        return hah_seek_team
    end
    
    if server.player_team(cn) ~= jointeam then    
        server_change_team_request[cn] = true 
        server.changeteam(cn, jointeam)
    end
    
    return jointeam
end

local function relocate_players()
    if server.is_valid_cn(hah_active_player) then
        hah_seek_team = server.player_team(hah_active_player)
    end
    for i, cn in ipairs(server.players()) do
        setteam(cn)
    end 
end

local function relocate_vars(set_seek)
    if server.is_valid_cn(hah_next_player) == 0 then
           
		while server.is_valid_cn(hah_next_player) == 0 do
			for i, cn_ in ipairs(server.players()) do
				if math.random(0, 1) == 1 then
					hah_next_player = cn_
				end
			end           
		end
				
			  
		server.msg(green() .. "Set random Player as Next Seek Player: " .. server.player_name(hah_next_player))
		
    end
	
	if seet_seek ~= nil then
		if set_seek == true then
			return 
		end
	end
    
    hah_active_player = hah_next_player 
        
    server.no_spawn(hah_active_player, 1)
    server.player_slay(hah_active_player)
    server_change_team_request[hah_active_player] = true 
    server.changeteam(hah_active_player, "seek")
    
    for i, cn_ in ipairs(server.players()) do
        if server.player_status(cn_) == "dead" and cn_ ~= hah_active_player then
            server.no_spawn(cn_, 0)
            server.player_slay(cn_)
            server.spawn_player(cn_)
        end
    end   
    
    relocate_players()
end

server.event_handler("spectator", function(cn, val)
	if hide_and_seek == false then return end
	if val == 0 then
		setteam(cn)
	end
end)

server.event_handler("connect", function(cn)
	if hide_and_seek == false then return end
	local str = "\f3PLEASE READ: "..orange().."This server is currently running the Hide and Seek mode, make sure you have the current map and understand how the mode works, before you are asking to be unspecced! thank you!"

	server.sleep(3000, function() server.player_msg(cn, str) end)
	server.sleep(5000, function() server.player_msg(cn, str) end)
	server.sleep(7000, function() server.player_msg(cn, str) end)

end)

server.event_handler("damage", function(client, actor)
    if hide_and_seek == false then return end
    if client == actor then
        return -1
    end
    if server.player_team(actor) == server.player_team(client) and actor ~= client then
        server.player_msg(actor, red() .. "You can't frag your teammates in this mode!")
        return -1
    end
    if server.player_team(actor) ~= hah_seek_team and actor ~= hah_active_player and client ~= actor then
        server.player_msg(actor, red() .. "You are not allowed to attack the seek Player!!!")
        return -1       
    end
end)

server.event_handler("frag", function(client, actor)
    if hide_and_seek == false then return end
    if hah_active_player == actor and client ~= actor then
        hah_actor_frags = hah_actor_frags + 1
        if hah_actor_frags == 1 then
            server.msg(red() .. string.upper("Next seek player set: \f1" .. server.player_name(client)))
            hah_next_player = client
        end
    end
    if client ~= actor then
        if hah_seek_team == server.player_team(actor) then
            local count = 0
            for i, cn_ in ipairs(server.clients()) do
                if server.player_status(cn_) ~= "spectator" and hah_seek_team ~= server.player_team(cn_) then
                    count = count + 1  
                end
            end
            count = count - 1
	    local str = "Players"
	    if count == 1 then
		str = "Player"
	    end
            server.msg(orange() .. server.player_name(actor) .. " got " .. server.player_name(client) .. " - " .. count .. " " .. str .. " left!")
            server_change_team_request[client] = true 
            server.changeteam(client, hah_seek_team)
            caught_players[client] = true
            if count == 0 then
                server.changetime(0)
            end
        end
    end
end)

server.event_handler("suicide", function(cn)
    if hide_and_seek == false then return end
    if server_change_team_request[cn] == true then return end
    if hah_seek_team ~= server.player_team(cn) then
        hah_actor_frags = hah_actor_frags + 1
        if hah_actor_frags == 1 then
            server.msg(blue() .. "Next seek player set: " .. server.player_name(cn))
            hah_next_player = cn
        end
        local count = 0
        for i, cn_ in ipairs(server.clients()) do
            if server.player_status(cn_) ~= "spectator" and hah_seek_team ~= server.player_team(cn_) then
                count = count + 1  
            end
        end
        count = count - 1
        server.msg(green() .. server.player_name(cn) .. " suicided and became a seeker - " .. count .. " Players left!")
        server_change_team_request[cn] = true 
        server.changeteam(cn, hah_seek_team)
        caught_players[cn] = true
        if count == 0 then
            server.changetime(0)
        end
    end
end)

server.event_handler("intermission", function(actor, client)
    if hide_and_seek == false then return end
       
    server.intermission = server.gamemillis + 10000
    
    local starttime = round((server.intermission - server.gamemillis))
    
    server.sleep((starttime - 10), function() -- riscy, but should work :P
        server.changemap(server.map)
    end)
end)

ALREADY_DONE = false

server.event_handler("mapchange", function()
	if hide_and_seek == false then return end
    	server.msg(green() .. "waiting for clients to load the map...")
	for i, cn in ipairs(server.clients()) do
		server.player_slay(cn)
		server.no_spawn(cn, 1)
	end
	reset_on_mapchange() 
end)

server.event_handler("spectator", function(cn, join)
    if hide_and_seek == false then return end
    if join == 0 then
        relocate_players()
    end
end)


server.event_handler("reteam", function(cn, old, new)
    if hide_and_seek == false then return end
    if server_change_team_request[cn] == true then
        server_change_team_request[cn] = false
        return
    end
    server_change_team_request[cn] = true 
    server.changeteam(cn, old)
    server.player_msg(cn, red() .. "You can't switch the team!")
end)

server.event_handler("mapvote", function(cn, map, mode)
	if hide_and_seek == false then return end
	if server.player_priv(cn) ~= "master" and server.player_priv(cn) ~= "admin" then
		server.player_msg(cn, red() .. "Only Master/Admin can set a Map!")
		return -1
	else	
		if can_vote == false then
			server.player_msg(cn, red() .. "Please wait until the seek player spawned for a new mapvote!")
			return -1
		else
			if mode ~= "teamplay" then
				server.player_msg(cn, red() .. "Hide and Seek can only be played in Teamplay-Mode!")
				return -1
			end
		end
	end
end)



server.event_handler("maploaded", function(cn)
    if hide_and_seek == false then return end
    if GAME_ACTIVE == true then
        return
    end
    
	PLAYER_ACTIVE[cn] = true

	local canstart = true
	for _, cn_ in ipairs(server.players()) do
		if PLAYER_ACTIVE[cn_] == nil then
			canstart = false 
		end
	end
    
    if canstart then
        GAME_ACTIVE = true
        
        relocate_vars()
        
        server.msg(red() .. "Maploading finished!")

		for i, cn_ in ipairs(server.clients()) do
			if cn_ ~= hah_active_player then
				server.no_spawn(cn_, 0)
				server.spawn_player(cn_)
			end
		end		

		server.sleep(0, function()
		
			server.msg(green() .. "Go and hide, the seek player will spawn in 10 seconds!")
			
			
			if server.player_status(hah_active_player) == "spectator" then
				 relocate_vars(true)
			end
        
			for i, cn_ in ipairs(server.players()) do
				if cn_ ~= hah_active_player then
					server.spawn_player(cn_)
				end
			end
			
			server.sleep(5000, function()
				server.msg(blue() .. "5 seconds, run!") 
			end)
			server.sleep(7000, function()
				server.msg(red() .. "3 seconds...") 
			end)
			server.sleep(8000, function()
				server.msg(red() .. "2 seconds...") 
			end)
			server.sleep(9000, function()
				server.msg(red() .. "1 second....") 
				stop_kill_event = true
			end)
			server.sleep(10000, function()
				server.no_spawn(hah_active_player, 0)
				server.spawn_player(hah_active_player)
				server.msg(yellow() .. "Seek Player spawned!")
				can_vote = true
				server.changetime(1000*60*has_time)
			end)
		
		end)
    end
end)

server.event_handler("disconnect", function(cn, reason)
    if server.playercount == 0 and hide_and_seek == true then
        server.mastermode = 0
        hide_and_seek = false
    end
end)

function server.playercmd_has(cn, enable)
    enable = tonumber(enable)
    if server.player_priv(cn) ~= "master" and server.player_priv(cn) ~= "admin" then
	server.player_msg(cn, red() .. "You need master/admin to enable/disable hide and seek!")
	return
    end
    if enable == 1 then
	server.broadcast_mapmodified = false
        hide_and_seek = true
	server.HIDE_AND_SEEK = 1
        server.mastermode = 2
        server.msg("mastermode is now locked (2)")
        server.msg(green() .. "Hide and Seek Mode enabled!")
	can_vote = true
    else
	server.broadcast_mapmodified = true
	server.HIDE_AND_SEEK = 0
        server.msg(blue() .. "Hide and Seek Mode disabled!")
        server.mastermode = 0
        hide_and_seek = false
    end
end
