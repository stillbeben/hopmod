-- (c) 2010 by Thomas

hide_and_seek = false
hah_active_player = -1
hah_seek_team = ""
hah_next_player = -1
hah_actor_frags = 0
MAP_LOADED_ALL = false
PLAYER_ACTIVE = { }
hah_active_player_can_spawn = false
GAME_ACTIVE = false
PLAYER_SWITCHED = false
hah_active_player_spawn_tries = 0
caught_players = { }
server_change_team_request = { } 

function reset_on_mapchange()
    hah_actor_frags = 0
    MAP_LOADED_ALL = false
    PLAYER_ACTIVE = { }
    hah_active_player_can_spawn = false
    GAME_ACTIVE = false
    PLAYER_SWITCHED = false
    hah_active_player_spawn_tries = 0
    caught_players = { }
    server_change_team_request = { } 
end

local function setteam(cn)
    local jointeam = ""
    
    if caught_players[cn] ~= nil or hah_active_player == cn then
        return hah_seek_team
    end
    
    if hah_seek_team == "good" then
        jointeam = "evil"
    else
        jointeam = "good"
    end
    
    if server.player_team(cn) ~= jointeam then    
        server_change_team_request[cn] = true 
        server.changeteam(cn, jointeam)
    end
    
    return jointeam
end

function server.relocate_players()
    if server.is_valid_cn(hah_active_player) then
        hah_seek_team = server.player_team(hah_active_player)
    end
    for i, cn in ipairs(server.players()) do
        setteam(cn)
    end 
end

function relocate_vars()
        if server.is_valid_cn(hah_next_player) == 0 then
            server.msg(red() .. "ATTENTION: Given Next Seek Player CN doesnt exist!")
            
            while server.is_valid_cn(hah_next_player) == 0 do
                for i, cn_ in ipairs(server.players()) do
                    if math.random(0, 1) == 1 then
                        hah_next_player = cn_
                    end
                end           
            end
            
          
            server.msg(green() .. "Set random Player as Next Seek Player: " .. server.player_name(hah_next_player))
        end
    
    hah_active_player = hah_next_player 
        
    server.no_spawn(hah_active_player, 1)
    server.player_slay(hah_active_player)
    
    for i, cn_ in ipairs(server.players()) do
        if server.player_status(cn_) == "dead" and cn_ ~= hah_active_player then
            server.no_spawn(cn_, 0)
            server.player_slay(cn_)
            server.spawn_player(cn_)
        end
    end   
    
    server.relocate_players()
end

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
            server.msg(blue() .. "Next seek player set: " .. server.player_name(client))
            hah_next_player = client
        end
    end
    if client ~= actor then
        if hah_seek_team == server.player_team(actor) then
            local count = 0
            for i, cn_ in ipairs(server.clients()) do
                if server.player_status ~= "spectator" and hah_seek_team ~= server.player_team(cn_) then
                    count = count + 1  
                end
            end
            count = count - 1
            server.msg(green() .. server.player_name(actor) .. " got " .. server.player_name(client) .. " - " .. count .. " Players left!")
            server_change_team_request[cn] = true 
            server.changeteam(client, hah_seek_team)
            caught_players[client] = true
            if count == 0 then
                server.changetime(0)
            end
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

server.event_handler("mapchange", function(cn)
	if hide_and_seek == false then return end
    server.msg(green() .. "waiting for clients to load the map...")
end)

server.event_handler("spectator", function(cn, join)
    if hide_and_seek == false then return end
    if join == 0 then
        server.relocate_players()
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
        
        reset_on_mapchange() 
        relocate_vars()
        
        server.msg(red() .. "Maploading finished, go and hide, the seek player will spawn in 10 seconds!")
        
        for i, cn_ in ipairs(server.clients()) do
            if server.player_status(cn_) ~= "spectator" and cn_ ~= hah_active_player then
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
        end)
    end
end)


function server.playercmd_hah(cn, enable)
    enable = tonumber(enable)
    if enable == 1 then
        hide_and_seek = true
        server.mastermode = 2
        server.msg("mastermode is now locked (2)")
        server.msg(green() .. "Hide and Seek Mode enabled!")
    else
        server.msg(blue() .. "Hide and Seek Mode disabled!")
        hide_and_seek = false
        return
    end
end

