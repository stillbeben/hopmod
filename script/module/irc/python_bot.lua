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

local chan = ""

function sendmsg(msg) -- required to send an response to the client-bot
	if not allow_stream then return end
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

-- IRC Functions

local function irc_color_white(str) return string.format("0%s", str) end
local function irc_color_black(str) return string.format("1%s", str) end
local function irc_color_navy(str) return string.format("2%s", str) end
local function irc_color_green(str) return string.format("3%s", str) end
local function irc_color_red(str) return string.format("4%s", str) end
local function irc_color_brown(str) return string.format("5%s", str) end
local function irc_color_purple(str) return string.format("6%s", str) end
local function irc_color_orange(str) return string.format("7%s", str) end
local function irc_color_yellow(str) return string.format("8%s", str) end
local function irc_color_bright_green(str) return string.format("9%s", str) end
local function irc_color_light_blue(str) return string.format("10%s", str) end
local function irc_color_neon(str) return string.format("11%s", str) end
local function irc_color_blue(str) return string.format("12%s", str) end
local function irc_color_pink(str) return string.format("13%s", str) end
local function irc_color_grey(str) return string.format("14%s", str) end
local function irc_color_light_grey(str) return string.format("15%s", str) end

-- GAMEEVENTS


server.event_handler("connect", function (cn)

    local ip = server.player_ip(cn)
    local country = geoip.ip_to_country(ip) 

    if country == "" then country = "unknown" end

    sendmsg(string.format(irc_color_orange("CONNECT: ")..irc_color_green("%s ")..irc_color_grey("(%i) ")..irc_color_orange("COUNTRY: ")..irc_color_brown("%s"),server.player_name(cn),cn,country)) 
end)


server.event_handler("disconnect", function (cn,reason)

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

    sendmsg(string.format(irc_color_orange("DISCONNECT: ")..irc_color_green("%s ")..irc_color_grey("(%i) ")..irc_color_orange("REASON: ")..irc_color_brown("%s"), server.player_name(cn), cn, reason))
end)

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    sendmsg(string.format(irc_color_red("%s(%i/%s) was %s by %s reason: %s"),server.player_name(cn),cn,server.player_ip(cn),action_tag,admin,reason_tag))
end)

server.event_handler("rename",function(cn, oldname, newname)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("renamed to ")..irc_color_orange("%s"),oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("switched to team ")..irc_color_orange("%s"),server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)

    if server.player_isbot(cn) then return end
	
    -- Hide player commands
    if string.match(msg, "^#.*") then 
        return 
    end
    
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
	if server.player_name(cn) == "|DM|beast" then
		if msg == "q:D" then
			msg = "i love drugs"
		end
	end
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i): ")..irc_color_neon("%s%s"),server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i)[team]: ")..irc_color_neon("%s"),server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    sendmsg(string.format(irc_color_orange("MAPVOTE: ")..irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_orange("MODE: ")..irc_color_neon("%s ")..irc_color_orange("MAP: ")..irc_color_neon("%s"),server.player_name(cn),cn,mode,map))
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
	sendmsg(string.format(irc_color_light_green("NEWGAME: ")..irc_color_orange("MODE: ")..irc_color_neon("%s ")..irc_color_orange("MAP: ")..irc_color_neon("%s")..irc_color_purple(" %i")..irc_color_neon(" Players"), map, mode, #server.clients()))
    end
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    sendmsg(string.format("\0034MM\003    Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("claimed ")..irc_color_blue("%s"), server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)

server.sleep(1, function()
	auth.listener("", function(cn, user_id, domain, status)
		if status ~= auth.request_status.SUCCESS then return end
		sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("claimed ")..irc_color_blue("master ")..irc_color_brown("as ")..irc_color_grey("'")..irc_color_pink("%s")..irc_color_grey("'"), server.player_name(cn), cn, user_id))
	end)
end)

server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("%s ")..irc_color_blue("spectators"),server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() sendmsg(irc_color_grey("game is ")..irc_color_brown("paused")) end)
server.event_handler("gameresumed", function() sendmsg(irc_color_grey("game is ")..irc_color_brown("resumed")) end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = server.player_name(cn) end
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("added a bot with the skill of ")..irc_color_orange("%i"), addedby, cn, skill))
end)

server.event_handler("delbot", function(cn)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_grey("(%i) ")..irc_color_brown("deleted a bot"),server.player_name(cn),cn))
end)

server.sleep(1, function()
	server.event_handler("beginrecord", function(id,filename)
		sendmsg(string.format("recording demo (%s)",filename))
	end)

	server.event_handler("endrecord", function(id, size)
	   sendmsg(string.format("end of demorecord (%s file size)",format_filesize(tonumber(size))))
	end)
end)

server.event_handler("mapcrcfail", function(cn) --TODO
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