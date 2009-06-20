
function admincmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_ADMIN) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function mastercmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_MASTER) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function server.playercmd_info(cn)
    server.player_msg(cn, orange(server.servername) .. ": " .. server.motd )
end

function server.playercmd_specall(cn)
    return admincmd(server.specall,cn)
end

function server.playercmd_unspecall(cn)
    return admincmd(server.unspecall,cn)
end

-- [[ Player commands written by Thomas

function server.playercmd_maxclients(cn, arg1) 
	return admincmd(
		function ()
			if not arg1 then server.player_msg(cn, red("missing maxclient argument")) return end
			if arg1 >= server.playercount and arg1 <= "128" then
				server.maxplayers = arg1
			end
		end
	,cn)
end

function server.playercmd_uptime(cn) 
	server.player_msg(cn, "Server-Uptime: " .. format_duration(server.uptime / 1000))
end

function server.playercmd_reload(cn_client) 
	return admincmd(function () server.reloadscripts() end, cn_client)
end

function server.playercmd_changetime(cn, arg1) 
	return admincmd(function () server.changetime(arg1*60*1000) end, cn)
end

function server.playercmd_lpc(cn) -- list player countrys 
	for i, player in ipairs(server.players()) do 
        server.player_msg(cn, "Player: " .. green(server.player_name(player)) .. " Country: " ..  orange(server.ip_to_country(server.player_ip(player))))
    end
end

function server.playercmd_players(cn_client) -- list players + player stats
    for i, cn in ipairs(server.players()) do 
        str = "Name: " .. server.player_name(cn) .. " Frags: " .. server.player_frags(cn) .. " Deaths: " .. server.player_deaths(cn) .. " Acc: " .. server.player_accuracy (cn)
        server.player_msg(cn_client, str)
    end
end

function server.playercmd_names(cn, target_cn)
    local db = sqlite3.open(server.stats_db_filename)
    local str = "Other names used by " .. server.player_name(target_cn) .. ": "
    for name, count in db:cols("SELECT DISTINCT name, count(name) as count FROM players WHERE ipaddr = '" .. server.player_ip(target_cn) .. "'") do
        str = str .. name .. "(" .. count .. "),"
    end
    server.player_msg(cn, str)
    db:close(db)
end

server.event_handler("started", function()

    if tonumber(server.enable_votekick_command) == 0 then return end
        
    local varvk = {}
    
    server.event_handler("connect", function (cn) varvk[cn] = {} end)
    server.event_handler("disconnect", function (cn, reason) varvk[cn] = nil end)
    
    local function isplayer(arg) 
        for i, cn in ipairs(server.players()) do 
            if tonumber(cn) == tonumber(arg) then return true end
        end
        return false
    end
     
    local function isnum(arg)
        if arg < "0" or arg > "9" then return false end
        return true
    end
    
    server.playercmd_votekick = function(cn, cn_kick)
    
        if not isnum(cn_kick) then return end  -- isnum = used from duel script
        if not isplayer(cn_kick) then return end -- isplayer = used from duel script
        if cn_kick == cn then return end -- dont kick yourself :)
        if tonumber(server.playercount) < 3 then return end -- min 3 players
        if varvk[cn_kick][cn] then -- already voted
            server.player_msg(cn, red("You have already voted to kick this player!"))
            return
        end
        
        varvk[cn_kick][cn] = true
        if varvk[cn_kick].votes == nil then varvk[cn_kick].votes = 0 end
        varvk[cn_kick].votes = varvk[cn_kick].votes + 1
        server.msg(green(server.player_name(cn)) .. " voted to kick " .. red(server.player_name(cn_kick)))
        local required_votes = round((server.playercount / 2), 0)
        server.msg("Votes: " .. varvk[cn_kick].votes .. " of " .. required_votes)
        if varvk[cn_kick].votes >= required_votes then
            server.kick(cn_kick, 3600, "server", "votekick") -- ban for 1 hour
            server.msg("Player was kicked by vote-kick.")
            varvk[cn_kick] = nil
        end
        
    end
    
end)

-- ]]

function server.playercmd_pause(cn)
    return admincmd(function ()
        server.pausegame(true)
    end, cn)
end

function server.playercmd_resume(cn)
    return admincmd(function ()
        server.pausegame(false)
    end, cn)
end

function server.playercmd_motd(cn, text)
    return admincmd(function (cn)
        server.motd = text
        server.player_msg(cn, "MOTD changed to " .. text)
    end, cn)
end

function server.playercmd_group(cn, tag_pattern, team)
    return admincmd(function ()
        
        if not server.gamemodeinfo.teams then return end
        
        if not tag_pattern then
            server.player_msg(red("missing tag pattern argument"))
            return
        end
        
        if not team then
            server.player_msg(red("missing team argument"))
            return
        end
        
        for i,cn in ipairs(server.spectators()) do
            if string.match(server.player_name(cn), tag_pattern) then server.unspec(cn) end
        end
        
        for player in server.gplayers() do
            
            local teamnow = player:team()
            
            if teamnow ~= team and string.match(player:name(), tag_pattern) then
            
                server.msg(string.format("Admin has moved %s to %s team.", green(server.player_name(cn)), green(team)))
            
                server.changeteam(cn, team)
            
            end
            
        end
        
    end, cn)
end

function server.playercmd_givemaster(cn, target)
    return mastercmd(function ()
        
        if server.player_id(target) == -1 then
            server.player_msg(cn, red("Player not found."))
            return
        end
        
        server.unsetmaster()
        
        server.player_msg(target, server.player_name(cn) .. " has passed master privilege to you.")
        server.setmaster(target)
        
    end, cn)
end

-- [[ Player commands written by ]Zombie[

function server.playercmd_mute(cn,mute_tcn)
    return mastercmd(function()
        if not mute_tcn then
            server.player_msg(cn,red("cn is missing"))
        else
            server.mute(mute_tcn)
        end
    end,cn)
end 

function server.playercmd_unmute(cn,unmute_tcn)
    return mastercmd(function()
        if not unmute_tcn then
            server.player_msg(cn,red("cn is missing"))
        else
            server.unmute(unmute_tcn)
        end
    end,cn)
end

function server.playercmd_ban(cn,ban_tcn,ban_time,ban_unit,ban_reason)
    return mastercmd(function()
        if not ban_tcn then
            server.player_msg(cn,red("cn is missing"))
            return
        end
        if not ban_time then
            ban_ltime = 60
        else
            ban_ltime = ban_time
        end
        if ( not ban_unit ) or ( ban_unit == "m" ) then
            ban_ltime = ban_ltime * 60
        elseif ban_unit == "h" then
            ban_ltime = ban_ltime * 3600
        elseif ban_unit == "s" then
            ban_ltime = ban_time
        elseif not ban_reason then
            ban_lreason = ban_unit
        else
            server.player_msg(cn,red("unknown time-unit..."))
        end
        if not ban_reason then
            if not ban_lreason then
                ban_lreason = " "
            end
        else
            ban_lreason = ban_reason
        end
        server.kick(ban_tcn,ban_ltime,cn,ban_lreason)
        ban_tcn = " "
        ban_ltime = " "
        ban_lreason = " "
    end,cn)
end 

function server.playercmd_kick(cn,kick_tcn,kick_reason)
    return mastercmd(function()
        if not kick_tcn then
            server.player_msg(cn,red("cn is missing"))
        elseif not kick_reason then
            server.kick(kick_tcn,"1",cn,"")
        else
            server.kick(kick_tcn,"1",cn,kick_reason)
        end
    end,cn)
end 

function server.playercmd_sd(cn, arg1)
        return admincmd(function () 
		sudden_death_enabled = "true" 
		server.msg(red("--[ Sudden Death Mode Enabled. There will be no ties"))
	end, cn)
end
function server.playercmd_nosd(cn, arg1)
        return admincmd(function () 
		sudden_death_enabled = "false" 
		sudden_death = "false"
		server.msg(red("--[ Sudden Death Mode Disabled. There will be ties"))
	end, cn)
end

server.event_handler("timeupdate", function (mins)
        if sudden_death_enabled == "true" then
                player_score = {}
                if endgame == "true" then endgame = ""; return 0 end
                if mins == "1" then
                        for index, cn in ipairs(server.players()) do
                                player_score[index] = server.player_frags(cn)
                        end
                        server.sleep(59000,function()
                                if player_score[1] == player_score[2] then
                                        server.msg(red("--[ Sudden Death. Next Frag Wins!"))
                                        sudden_death = "true"
                                end
                        end)
                end

        end     if sudden_death == "true" then return 1 else return mins end
end)

server.event_handler("frag", function (target,actor)
        if sudden_death_enabled == "true" then
                if sudden_death == "true" then
                        endgame = "true"
                        sudden_death = "false"
                        server.changetime(0)
                end
        end
end)

-- 1on1 script 
-- Version: 0.1
-- (c) by Thomas
-- #1on1 cn1 cn2 mode map
 
var1on1 = false
var1on1player1 = "" -- cn
var1on1player1map = false
var1on1player2 = "" -- cn
var1on1player2map = false
var1on1gamecount = 0
 
 
server.event_handler("active", function (cn) 
        if var1on1 then
                if cn == var1on1player1 then 
                        var1on1player1map = true
                end
                if cn == var1on1player2 then 
                        var1on1player2map = true
                end
                if var1on1player1map and var1on1player2map then
                        server.msg(orange("--[ Game started! Happy Fragging."))
                        server.pausegame(false)
                        var1on1player1map = false
                        var1on1player2map = false
                end
        end 
end)
 
server.event_handler("mapchange", function (map, mode) 
        if var1on1 then
                var1on1gamecount = var1on1gamecount + 1
                if var1on1gamecount > 1 then
                        var1on1gamecount = 0
                        var1on1 = false
                        return
                end
                server.pausegame(true)
                server.recorddemo()
                server.msg(orange("--[ Starting Demo Recording"))
                server.msg(orange("--[ Waiting until all Players loaded the Map."))
        end 
end)
 
server.event_handler("intermission", function () 
        if var1on1 then
                if server.player_frags(var1on1player1) > server.player_frags(var1on1player2) then
                        server.msg(green("--[ 1on1 Game ended - " .. blue(server.player_name(var1on1player1)) .. " won the Game!"))       
                end
                if server.player_frags(var1on1player1) < server.player_frags(var1on1player2) then
                        server.msg(green("--[ 1on1 Game ended - " .. blue(server.player_name(var1on1player2)) .. " won the Game!"))
                end
                if server.player_frags(var1on1player1) == server.player_frags(var1on1player2) then
                        server.msg("--[ 1on1 Game ended - No Winner!")
                end
                var1on1 = false
        end
end)

server.event_handler("connect", function (cn)
        if server.player_name(cn) .. server.player_ip(cn) == var1on1player1_id then
		server.unspec(cn)
		server.sleep(0,    function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 5")) end)
                server.sleep(1000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 4")) end)
                server.sleep(2000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 3")) end)
                server.sleep(3000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 2")) end)
                server.sleep(4000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 1")) end)
                server.sleep(5000, function() server.pausegame(false) end)
        end
        if server.player_name(cn) .. server.player_ip(cn) == var1on1player2_id then
		server.unspec(cn)
		server.sleep(0,    function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 5")) end)
                server.sleep(1000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 4")) end)
                server.sleep(2000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 3")) end)
                server.sleep(3000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 2")) end)
                server.sleep(4000, function() server.msg(orange("--[ Opponent " .. blue(server.player_name(cn)) .. " Reconnected Resuming Game in 1")) end)
                server.sleep(5000, function() server.pausegame(false) end)
        end
end)

server.event_handler("disconnect", function (cn)
	if server.player_name(cn) .. server.player_ip(cn) == var1on1player1_id then 
		server.msg(red("--[ Opponent " .. blue(server.player_name(cn)) .. " Disconnected Pausing Game"))
		server.pausegame(true)
	end 
	if server.player_name(cn) .. server.player_ip(cn) == var1on1player2_id then 
		server.msg(red("--[ Opponent " .. blue(server.player_name(cn)) .. " Disconnected Pausing Game")) 
		server.pausegame(true)
	end
end)
 
function isplayer(arg) 
    for i, cn in ipairs(server.players()) do 
                if tonumber(cn) == tonumber(arg) then return true end
    end
    for i, cn in ipairs(server.spectators()) do
                if tonumber(cn) == tonumber(arg) then return true end
    end
        return false
end
 
function isnum(arg)
        if arg < "0" or arg > "9" then return false end
        return true
end
 
function server.playercmd_versus(cn, player1, player2, mode, map) 
        if tonumber(server.player_priv_code(cn)) == 0 then return end
        if var1on1 then return end
        if not isnum(player1) or not isnum(player2) then return end
        if not isplayer(player1) or not isplayer(player2) then return end
        if player1 == player2 then return end
        var1on1 = true
        var1on1gamecount = 0
        var1on1player1 = player1
        var1on1player2 = player2
	var1on1player1_id = server.player_name(player1) .. server.player_ip(player1)
	var1on1player2_id = server.player_name(player2) .. server.player_ip(player2)
        server.msg(green("--[ 1on1 - " .. server.player_name(player1) .. " against " .. server.player_name(player1) .. " mode: " .. mode  .. " map: " .. map ))
        server.specall()
        server.unspec(player1)
        server.unspec(player2)
        server.mastermode = 2
        server.pausegame(true)
        server.sleep(0, function() server.msg(orange(	"--[ Loading the Map in 5 Seconds")) end)
        server.sleep(1000, function() server.msg(orange("--[ Loading the Map in 4 Seconds")) end)
        server.sleep(2000, function() server.msg(orange("--[ Loading the Map in 3 Seconds")) end)
        server.sleep(3000, function() server.msg(orange("--[ Loading the Map in 2 Seconds")) end)
        server.sleep(4000, function() server.msg(orange("--[ Loading the Map in 1 Second ")) end)
        server.sleep(5000, function() server.changemap(map, mode, -1) end)
end
-- 1on1 script end

-- ]]
