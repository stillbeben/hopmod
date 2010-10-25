--[[

HopMod Module for the Python-IRC-Bot
(c) 2009-2010 by Thomas, PunDit, Michael

Please note: This Module is _very_ beta, and needs a lot of improvement.

The SocketListener will wait on the serverport + 10 (for example 28785 + 10 = 28795) for the bot

--]]

if server.irc_socket_password == nil then
	server.irc_socket_password = ""
end

server.sleep(1000, function() -- wait a second before starting bot-listener

if server.irc_socket_password == "" then error("please set a password for the ircbot listener!") return end

require("net")

local acceptor = net.tcp_acceptor("0.0.0.0", server.serverport+10)
acceptor:listen()

local client_bot = net.tcp_client()

local chan 

function sendmsg(msg) -- required to send an response to the client-bot
    if chan ~= "" then
		chan = chan .. " "
    end
    client_bot:async_send(chan..msg.."\r\n", function(success) end)
    chan = ""
end

local allow_stream = false

local function process_irc_command(data)
      local data = string.sub(data, 0, string.len(data)-2) -- skip \n
      if string.find(data, "pass:") then
	    local pass = string.gsub(data, "pass:", "")
	    if pass ~= server.irc_socket_password then
			allow_stream = false
			sendmsg("pass:wrong password")
			client_bot:close()
	    else
			allow_stream = true
	    end
      end 


      if not allow_stream == true then return end

      -- DO NOT ADD ANYTHING ABOVE!!!! 


      if string.find(data, "code:") then
		  local xchan = Split(data, " ")
		  chan = xchan[1]
		  
		  local tmp = ""
		  for i, t in ipairs(xchan) do
			  if i > 1 then
			tmp = tmp .. " " .. t
			  end
		  end
		
		  local code = string.gsub(tmp, "code:", "")


		  local pcallret, success, errmsg = pcall(loadstring(code))
		  if success ~= nil then
			sendmsg("error while executing code: "..success)
			if dm_log ~= nil then
				log("irc error -> " .. success) 
			end
		  else
			  log(string.format("irc -> executed: [[ %s ]]", code))
		  end
      end

    if data == "ping" then
		sendmsg("pong")
    end
end


local function ircreadloop()
  client_bot:async_read_until("\n", function(data)
     if data then
         process_irc_command(data)
	 ircreadloop()
     end
  end)
end

local function accept_next(acceptor)
    acceptor:async_accept(function(client)

	client_bot = client

	--sendmsg("1:connection accepted, waiting for password.")
	allow_stream = false
	ircreadloop()
        
        accept_next(acceptor)
    end)
end

accept_next(acceptor)

end)


-- ADD GAMEEVENTS HERE


server.event_handler("connect", function (cn)

    if logged_in_spy[tonumber(cn)] == true then return end

    local ip = server.player_ip(cn)
    local country = geoip.ip_to_country(ip) 

    if country == "" then country = "unknown" end

    sendmsg(string.format("7CONNECT: 10%s(%i)7 Country:10 %s",server.player_name(cn),cn,country)) 
end)

server.event_handler("disconnect", function (cn,reason)

    if logged_in_spy[tonumber(cn)] == true then return end

    local reason_tag = ""
    local ip = ""

    if reason ~= "normal" then
        ip = "(" .. server.player_ip(cn) .. ")"
        reason_tag = " because: " .. reason
    end

    if server.player_connection_time(cn) < 2 then
	return
    end

    if reason == "" then
		reason = "normal"
    end

    sendmsg(string.format("7DISCONNECT: 10%s(%i) 3- 7REASON: 10%s", server.player_name(cn), cn, reason))
end)

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    sendmsg(string.format("4KICK: 14%s(%i/%s) 4was %s by %s reason: %s",server.player_name(cn),cn,server.player_ip(cn),action_tag,admin,reason_tag))
end)

server.event_handler("rename",function(cn, oldname, newname)
    sendmsg(string.format("4%s(%i)14 renamed to 4%s",oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    sendmsg(string.format("14TEAMCHANGE: %s(%i) -> %s",server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)

    if server.player_isbot(cn) then return end
	
    -- Hide player commands
    if string.match(msg, "^#.*") then 
        return 
    end
    
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
    sendmsg(string.format("9%s(%i): %s %s",server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    sendmsg(string.format("9%s(%i)//team:0 %s",server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    sendmsg(string.format("\0033MAPVOTE\003    \00312%s(%i)\003 suggests \0037%s\003 on map \0037%s",server.player_name(cn),cn,mode,map))
end)

local MAPNAME = ""

server.event_handler("mapchange", function(map, mode)
    MAPNAME = map
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = " " .. #server.clients() .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    if sendmsg ~= nil then
	sendmsg(string.format("3NEWGAME: Map: %s Mode: %s. %i Players.", map, mode, #server.clients()))
    end
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    sendmsg(string.format("\0034MM\003    Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    sendmsg(string.format("\0034MASTER\003    \00312%s(%i)\003 %s \0037%s\003", server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)

server.event_handler("user_auth", function(name, cn, user_id)
    sendmsg(string.format("\0034AUTH/MASTER\003    \00312%s(%i)\003 claimed master as %s \0037%s\003", server.player_name(cn), cn, user_id))
end)


server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    sendmsg(string.format("\0034SPEC\003    \00312%s(%i)\003 %s spectators",server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() sendmsg("\0034PAUSE\003    game is paused")end)
server.event_handler("gameresumed", function() sendmsg("\0034RESM\003    game is resumed") end)

server.event_handler("user_login", function(cn, level, group, id, name)
	if logged_in_user[cn] == "" then
		sendmsg(string.format("0%s(%i) 7logged in", server.player_name(cn), cn))
	end
end)

server.event_handler("stats_done", function(millis, ts)
    sendmsg("7wrote stats(" .. millis .. "ms) => http://dm-clan.ath.cx/scoreboard/details.php?game="..ts)
end)


server.event_handler("user_logout", function(cn)
    sendmsg(string.format("0%s(%i) 7logged out.", server.player_name(cn), cn))
end)



server.event_handler("user_login_failed", function(cn, level, group, id, name)
   sendmsg(string.format("%s failed to login", server.player_name(cn)))
end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = "\00312" .. server.player_name(cn) .. string.format("(%i)\003", cn) end
    sendmsg(string.format("\00315ADDBOT\003    %s added a bot (skill %i)", addedby, skill))
end)

server.event_handler("delbot", function(cn)
    sendmsg(string.format("\00315DBOT\003    \00312%s(%i)\003 deleted a bot",server.player_name(cn),cn))
end)

server.event_handler("beginrecord", function(id,filename)
    --sendmsg(string.format("\00312DEMOSTART\003    Recording game to %s",filename))
end)

server.event_handler("endrecord", function(id, size)
   -- sendmsg(string.format("\00312DEMOEND\003    finished recording game (%s file size)",format_filesize(tonumber(size))))
end)

server.event_handler("mapcrcfail", function(cn) 
    sendmsg(string.format("4%s(%i)\003 has a modified map.",server.player_name(cn),cn, server.map, server.player_mapcrc(cn), server.player_ip(cn)))
end)





local function get_best_stats(time)
    local players = server.clients()
    table.sort(players, function(a, b) return server.player_frags(a) > server.player_frags(b) end)
    local msg = ""
    local str = ""
    if time > 0 then
        str = "GAME STATS"
    else
        str = "GAME ENDED"
    end
    msg = "7" .. str .. ": ([" .. (server.gamemode or "unknown") .. "/" .. (server.map or "unknown") .. "]/".. #players .. " Players) BEST PLAYERS: ["
    for i, cn in ipairs(players) do 
        if i > 2 then break end
        local format = ""
		local health = ""
		if server.gamemode == "ctf" or server.gamemode == "ffa" then
			health = "/(" .. server.player_health(cn) .. "/" .. server.player_maxhealth(cn) .. ")"
		end
        if i == 1 then format = "" end
        if i == 2 then msg = msg .. "/ " end
        msg = msg .. string.format("%s%s (%i/%i/%i%s%s)%s", format, server.player_name(cn), server.player_frags(cn), server.player_deaths(cn), server.player_accuracy(cn), "%", health, format)
        if i == 2 then 
            msg = msg .. "] " 
        else
            msg = msg .. " "
        end
    end
    if #players == 0 then
        msg = msg .. "No Players] "
    end
    if #players == 1 then msg = msg .. "]" end
    if time > 0 then 
        msg = msg .. " TIME REMAINING: " .. time .. " Minutes"
    end
    return msg
end

server.event_handler("timeupdate", function(time) -- post game stats each minute
    if #server.clients() >= 1 then
        sendmsg(get_best_stats(time))
    end
    return -1
end)