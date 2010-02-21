--[[
   
	Allows the server to be monitored and administered from IRC. This is 
	experimental and will change quite a bit.
    
    Copyright (C) 2009	Charles Christopher(PunDit) 
    
    MAINTAINERS
       	PunDit 
        
    GUIDELINES
        * Watch out for message flooding
        
    TODO
		* Message throttling
		* Autorejoin Channels	
		* Support multiple channels
		* Support for all hopmod events
		* Command responses for commands with no event
		
]]

server.sleep(10000, function()

require "geoip"

irc = {}
irc.client = net.tcp_client()

if server.irc_debug == 1 then print ("DEBUG [Start UP] : Starting hopbot") end

--Connect and send required handshake information.
function irc:connectServer(client)
	irc.client:close()
	irc.client = net.tcp_client()
	irc.client:async_connect(server.irc_network, server.irc_network_port, function(errmsg) 
		if errmsg then irc:handleError(errmsg) return  end
		irc.client:async_send("NICK "..server.irc_bot_name.."\n", function(errmsg)
			if errmsg then irc:handleError(errmsg) return  end
			irc.client:async_send("USER  "..server.irc_network_username..string.random(15,"%l%d").." 8 *  : HopBot\n", function(errmsg)
				if errmsg then irc:handleError(errmsg) return  end
			end)
		if server.irc_debug == 1 then print ("DEBUG [connectServer] : Connected to "..server.irc_network) end
		irc:readData(irc.client) 
		end)
	end)
end

--Main Data Loop
function irc:readData(client)
	irc.client:async_read_until("\n", function(data)
        if data then 
            irc:processData(client, data)
            irc:readData(irc.client)
        end
	end)
end

--Process data read from the server.
function irc:processData(client,data)
	if server.irc_debug == 1 then print("DEBUG [Server Output] : "..data) end
	-- If a ping is detected respond with a pong
	if data.find(data,"PING") then
		local pong = string.gsub(data,"PING","PONG",1)
		if server.irc_debug == 1 then print ("DEBUG [processData:PING] : "..pong) end
		irc.client:async_send(pong,function(errmsg) end)
		return
	end
	-- If mode changes in channel then assume operator status has changed. Request userlist
	if data.find(data,"MODE") then
		irc.client:async_send("NAMES "..server.irc_channel.."\n",function(errmsg) end)
        return
    end
	-- Parse user list and store it as the operator list
	if data.find(data,"353") then
        irc.operators = string.match(data,"%S* 353 (.*)")
        return
    end
	-- After a notice is detected then start the join operation
	if data.find(data,"NOTICE") then
		server.sleep(10000, function()
		irc.client:async_send("JOIN "..server.irc_channel.."\n",function(errmsg) end)
		end) 
		return
	end 
	-- If the link is closed start over
	if data.find(data,"Closing Link") then
		irc:handleError('Link Closed','30000')
		return
	end
	-- Detect a command and hand it off to the command processor
	if string.match(data,":(.+)!.+ PRIVMSG (.+) :"..server.irc_bot_command_name.." (.+)") then
		nick,channel,command = string.match(data,":(.+)!.+ PRIVMSG (.+) :"..server.irc_bot_command_name.." (.+)")
		if server.irc_debug == 1 then print ("DEBUG [processData] : Nick: "..nick.." Channel: "..channel.." Command: "..command) end 
		irc:processCommand(nick,channel,command)
		return
	end
end 

--Process any commands
function irc:processCommand(nick,channel,command)
	cmd,arg1,arg2,arg3 = string.match(command, "(%S+)%s*(%S*)%s*(%S*)%s*(%S*)") 
	if not cmd 	then cmd 	= "null" end
	if not arg1 then arg1 	= "null" end
	if not arg2 then arg2 	= "null" end
	if not arg3 then arg3 	= "null" end
	if server.irc_debug == 1 then print ("DEBUG [processCommand] : Command: "..cmd.." Nick: "..nick.." Arg1: "..arg1.." Arg2: "..arg2.." Arg3: "..arg3) end
	if not string.match(irc.operators, "@"..nick) then return end
	irc.commands:case(cmd) -- Case command selector
	return
end

--Command to display players
function irc:playerList()
	player_list = {}
	counter = nil 
	output = "" 
		for player in server.aplayers() do
			if counter == nil then counter = 0 end
			counter = counter + 1
			output = output .. string.format("\00312%s\003(%s) ", player:displayname(), player.cn)
			if counter == 5 then irc:toChannel("\0036IRC\003         \0034-/WHO/-\003 is "..output); counter = 0 end
		end

		if counter == nil then 
			irc:toChannel("Apparently no one is connected.") 
		else
			if counter < 5 and counter > 0 then irc:toChannel("\0036IRC\003         \0034-/WHO/-\003 is "..output); end
		end
	return
	
end

--Wrap command in a pcall to stop it from killing the server. 
function irc:wrapCommand(command)
	local status, message = pcall(command)
		
	if status == false then
		err = string.match(message,"%w+:%w+:%s(.+)")
		if server.irc_debug == 1 then print("DEBUG [wrapCommand] : "..message); irc:toChannel(string.format("\0036IRC\003         \0034ERROR\003  \0034%s\003",err))  end
	end
	return
end

--Send message to channel.
function irc:toChannel(message)
	if message then
		irc.client:async_send( "PRIVMSG "..server.irc_channel.." : "..message.."\n", function(errmsg)  end)
		return
	end	
	return
end

--Handle any errors.
function irc:handleError(errmsg, time)
	if not errmsg then return end
	irc.client:cancel()
	irc.client:close()
    if server.irc_debug == 1 then print("DEBUG [handleError] : "..errmsg) end
	if not time then time = 30000 end
	server.sleep(time, function() 
		irc:connectServer(irc.client)
	 end)
end

--Case function
function switch(t)
	t.case = function (self,x)
		local f=self[x] or self.default
		if f then
			if type(f)=="function" then
				f(x,self)
			else
				error("case "..tostring(x).." not a function")
			end
		end
	end
	return t
end


-- Initiate Connection
irc:connectServer(irc.client)


-- Handle Game Message Events
server.event_handler("connect", function (cn)
    local ip = server.player_ip(cn)
    local country = geoip.ip_to_country(ip)
		irc:toChannel(string.format("\0039CONNECT\003    \00312%s(%i)\003 \0037%s\003",server.player_name(cn),cn,country)) 
end)

server.event_handler("disconnect", function (cn,reason)

    local reason_tag = ""
    local ip = ""

    if reason ~= "normal" then
        ip = "(" .. server.player_ip(cn) .. ")"
        reason_tag = " because: " .. reason
    end

    irc:toChannel(string.format("\0032DISCONNECT\003    \00312%s(%i)\003%s disconnected%s, time %s\n", server.player_name(cn), cn, ip, reason_tag, server.format_duration(server.player_connection_time(cn))))
end)




local function log_usednames(cn)

    if server.find_names_by_ip then
        local current_name = server.player_name(cn)
        local names = server.find_names_by_ip(server.player_ip(cn), current_name)
        
        local namelist = ""
        
        for index, name in ipairs(names) do
            local sep = ""
            if #namelist > 0 then sep = ", " end
            namelist = namelist .. sep .. name
        end
        
        irc:toChannel(string.format("\0039NAMES\003    Names used by \00312%s(%i)\003: %s\n", current_name, cn, namelist))
    end
   
end

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    irc:toChannel(string.format("\0034KICK\003    \00312%s(%i)\003 was \0037%s\003 by \0037%s\003 \0037%s\003\n",server.player_name(cn),cn,action_tag,admin,reason_tag),function(errmsg) end)
end)

server.event_handler("rename",function(cn, oldname, newname)
    irc:toChannel(string.format("\0032RENAME\003  \00312%s(%i)\003 renamed to \0037%s\003\n",oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    irc:toChannel(string.format("\0034CHANGETEAM\003    \00312%s(%i)\003 changed team to \0037%s\003\n",server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)
	if string.match(msg,"^#.*") then return end
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
    irc:toChannel(string.format("\0033CHAT\003    \00312%s(%i)\003%s  ~>  \0033%s\003\n",server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    irc:toChannel(string.format("\0033TEAMCHAT\003    \00312%s(%i)\003(team): %s\n",server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    irc:toChannel(string.format("\0033VOTE\003    \00312%s(%i)\003 suggests \0037%s\003 on map \0037%s\003",server.player_name(cn),cn,mode,map))
end)

server.event_handler("mapchange", function(map, mode)
    
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = tostring(pc) .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    irc:toChannel(string.format("\0032NEWMAP\003    New game: \0037%s\003 on \0037%s\003, \0037%s\003", mode, map, playerstats))
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    irc:toChannel(string.format("\0034MM\003    Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    irc:toChannel(string.format("\0034MASTER\003    \00312%s(%i)\003 %s \0037%s\003", server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)


server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    irc:toChannel(string.format("\0034SPEC\003    \00312%s(%i)\003 %s spectators",server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() irc:toChannel("\0034PAUSE\003    game is paused")end)
server.event_handler("gameresumed", function() irc:toChannel("\0034RESM\003    game is resumed") end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = "\00312" .. server.player_name(cn) .. string.format("(%i)\003", cn) end
    irc:toChannel(string.format("\00315ADDBOT\003    %s added a bot (skill %i)", addedby, skill))
end)

server.event_handler("delbot", function(cn)
    irc:toChannel(string.format("\00315DBOT\003    \00312%s(%i)\003 deleted a bot\n",server.player_name(cn),cn))
end)

server.event_handler("beginrecord", function(id,filename)
    irc:toChannel(string.format("\00312DEMOSTART\003    Recording game to %s",filename))
end)

server.event_handler("endrecord", function(id, size)
    irc:toChannel(string.format("\00312DEMOEND\003    finished recording game (%s file size)\n",format_filesize(tonumber(size))))
end)

server.event_handler("mapcrcfail", function(cn) 
    irc:toChannel(string.format("\0034MCRC\003    \00312%s(%i)\003 has a modified map (%s %i). [ip: %s]\n",server.player_name(cn),cn, server.map, server.player_mapcrc(cn), server.player_ip(cn)))
    log_usednames(cn)
end)

server.event_handler("shutdown", function() irc:toChannel("\0034HALT\003    Server shutting down"); end)

server.event_handler("reloadhopmod", function() irc:toChannel("\0034RELOAD\003    Reloading hopmod...\n") end)

irc:toChannel("\00312START\003    Server started")

-- Auth Listener for auth events
auth.listener("", function(cn, user_id, domain, status)
    if status ~= auth.request_status.SUCCESS then return end
    local msg = string.format("\00312%s(%i)\003 successfully authed", server.player_name(cn), cn)
    irc:toChannel(" \0034AUTH\003    " .. msg)
end)



-- Random sequence generator
local Chars = {}
for Loop = 0, 255 do
   Chars[Loop+1] = string.char(Loop)
end
local String = table.concat(Chars)

local Built = {['.'] = Chars}

local AddLookup = function(CharSet)
   local Substitute = string.gsub(String, '[^'..CharSet..']', '')
   local Lookup = {}
   for Loop = 1, string.len(Substitute) do
       Lookup[Loop] = string.sub(Substitute, Loop, Loop)
   end
   Built[CharSet] = Lookup

   return Lookup
end


function string.random(Length, CharSet)
   -- Length (number)
   -- CharSet (string, optional); e.g. %l%d for lower case letters and digits
   local CharSet = CharSet or '.'
   if CharSet == '' then
      return ''
   else
      local Result = {}
      local Lookup = Built[CharSet] or AddLookup(CharSet)
      local Range = table.getn(Lookup)
      math.randomseed( os.time() )
      for Loop = 1,Length do

         Result[Loop] = Lookup[math.random(1, Range)]
      end

      return table.concat(Result)
   end
end


--Command list
irc.commands = switch {
        ['kick']        = function () irc:wrapCommand(function() server.kick(arg1, 0,"IRC Admin", "Later Sucka") end) end,--
        ['version']     = function () irc:toChannel("HopBot v.l01 support #hopmod@irc.gamesurge.net") end,--
        ['spec']        = function () irc:wrapCommand(function() server.spec(arg1) end) end,--
        ['unspec']      = function () irc:wrapCommand(function() server.unspec(arg1) end) end,--
        ['mute']        = function () irc:wrapCommand(function() server.mute(arg1) end) end,--
        ['unmute']      = function () irc:wrapCommand(function() server.unmute(arg1) end) end,--
        ['setmaster']   = function () irc:wrapCommand(function() server.setmaster(arg1) end) end,--
        ['setadmin']    = function () irc:wrapCommand(function() server.setadmin(arg1) end) end,--
        ['unsetmaster'] = function () irc:wrapCommand(function() server.unsetmaster() end) end,--
        ['slay']        = function () irc:wrapCommand(function() server.player_slay(arg1) end) end,--
        ['changeteam']  = function () irc:wrapCommand(function() server.changeteam(arg1,arg2) end) end,--
        ['pause']       = function () irc:wrapCommand(function() server.pausegame(true) end) end,--
        ['unpause']     = function () irc:wrapCommand(function() server.pausegame(false) end) end,--
        ['map']         = function () irc:wrapCommand(function() server.changemap(arg1,arg2,"10") end) end,--
        ['delbot']     	= function () irc:wrapCommand(function() server.delbot(arg1) end) end,--
        ['restart']		= function () irc:wrapCommand(function() server.restart() end) end,--
        ['kill']		= function () irc:wrapCommand(function() server.restart_now() end) end,--
        ['shutdown']	= function () irc:wrapCommand(function() server.shutdown() end) end, --
        ['reload']		= function () irc:wrapCommand(function() server.reloadscripts() end) end,--
        ['say']			= function () irc:wrapCommand(function() local chat = string.match(command, "say%s*(.+)\n"); server.msg(red("Console").."("..nick.."): "..green(chat)); irc:toChannel("\003\0036IRC\003         \0034-/SAY/-\003 Console("..nick.."): \0034"..chat.."\003") end) end,--
        ['clearbans']	= function () irc:wrapCommand(function() server.clearbans() end) end,--
        ['permban']		= function () irc:wrapCommand(function() server.permban(arg1) end) end,--
        ['unsetban']	= function () irc:wrapCommand(function() server.unsetban(arg1) end) end,--
        ['recorddemo']	= function () irc:wrapCommand(function() server.recorddemo(arg1) end) end,--
        ['stopdemo']	= function () irc:wrapCommand(function() server.stopdemo() end) end,--
        ['who']			= function () irc:playerList() end,
        default = function () end,
        }



end)
