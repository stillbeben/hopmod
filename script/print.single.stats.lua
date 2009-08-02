-- print name
local function statsprinter_name()
    local file = io.open("log/stats/name.html","w+")
    local name = server.servername 
    if #name == 0 then
	name = "<EMPTY>"
    end
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=3>" .. name .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

-- print mastermode
local function statsprinter_mastermode(new)
    local file = io.open("log/stats/mastermode.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=1>" .. new .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()    
end

-- print host [ip port]
local function statsprinter_host()
    local file = io.open("log/stats/host.html","w+")
    local host = server.serverip
    if #host == 0 then
	host="0.0.0.0"
    end
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=1>" .. host .. " " .. server.serverport .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end
	
-- print playercount [players/maxplayers]
local function statsprinter_playercount()
    local file = io.open("log/stats/playercount.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=1>" .. server.playercount .. "/" .. server.maxplayers .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

-- print playerlist
local function statsprinter_players()
    local file = io.open("log/stats/players.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<table>\n")
    local gm = tostring(server.gamemode)
    if not ( gm == "instagib" or gm == "tactics" or gm == "ffa" or gm == "efficiency" ) then
	file:write("<tr><td width=\"50%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>name</i></font></td><td width=\"25%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>co</i></font></td><td width=\"25%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>team</i></font></td></tr>\n")
    else
	file:write("<tr><td width=\"75%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>name</i></font></td><td width=\"25%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>co</i></font></td></tr>\n")
    end
    for p in server.gplayers() do
	local country = server.ip_to_country_code(p:ip())
	if #country == 0 then
	    country = "??"
	end
	local status_player = ""
	if not ( gm == "instagib" or gm == "tactics" or gm == "ffa" or gm == "efficiency" ) then
	    file:write("<tr><td width=\"50%\"><font face=\"verdana\" size=1>" .. p:name() .. "</font></td><td width=\"25%\"><font face=\"verdana\" size=1>" .. country .. "</font></td><td width=\"25%\"><font face=\"verdana\" size=1>" .. p:team() .. "</font></td></tr>\n")
	else
	    file:write("<tr><td width=\"75%\"><font face=\"verdana\" size=1>" .. p:name() .. "</font></td><td width=\"25%\"><font face=\"verdana\" size=1>" .. country .. "</font></td></tr>\n")
	end
    end
    file:write("</table>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

-- print spectators
local function statsprinter_players_spec()
    local file = io.open("log/stats/spectators.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<table>\n")
    file:write("<tr><td width=\"75%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>name</i></font></td><td width=\"25%\"><font color=\"#c0c0c0\" face=\"verdana\" size=1><i>co</i></font></td></tr>\n")
    for q,cn in ipairs(server.spectators()) do
	local country = server.ip_to_country_code(server.player_ip(cn))
	if #country == 0 then
	    country = "??"
	end
	file:write("<tr><td width=\"75%\"><font face=\"verdana\" size=1>" .. server.player_name(cn) .. "</font></td><td width=\"25%\"><font face=\"verdana\" size=1>" .. country .. "</font></td></tr>\n")
    end
    file:write("</table>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

-- print gamemode
local function statsprinter_mode()
    local file = io.open("log/stats/mode.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=1>" .. server.gamemode .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

-- print map
local function statsprinter_map()
    local file = io.open("log/stats/map.html","w+")
    file:write("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n")
    file:write("<html>\n")
    file:write("<body text=\"#ffffff\">\n")
    file:write("<font face=\"verdana\" size=1>" .. server.map .. "</font>\n")
    file:write("</body>\n")
    file:write("</html>\n")
    file:flush()
end

local function statsprinter(job,option)
    if job == "name" then
	statsprinter_name()
    elseif job == "mastermode" then
	lmmode = "unknown"
	if not option then
	    lmm = tonumber(server.mastermode)
	    if lmm == 0 then
		lmmode = "open"
	    elseif lmm == 1 then
		lmmode = "veto"
	    elseif lmm == 2 then
		lmmode = "locked"
	    elseif lmm == 3 then
		lmmode = "private"
	    end
	else
	    lmmode = option
	end
	statsprinter_mastermode(lmmode)
    elseif job == "host" then
	statsprinter_host()
    elseif job == "players" then
	if not option then
	    statsprinter_playercount()
	elseif option == "active" then
	    statsprinter_players()
	elseif option == "spec" then
	    statsprinter_players_spec()
	end
    elseif job == "mode" then
	statsprinter_mode()
    elseif job == "map" then
	statsprinter_map()
    end
end

-- init on start
statsprinter("name")
statsprinter("host")
statsprinter("mode")
statsprinter("map")
statsprinter("players","spec")
server.sleep(1000,function()
    statsprinter("players")
    statsprinter("players","active")
    statsprinter("mastermode")
end)

-- rewrite mode, map at mapchange
-- additionaly, rewrite mastermode, when server goes empty
server.event_handler("mapchange",function(map,mode)
    statsprinter("mode")
    statsprinter("map")
    if tonumber(server.playercount) == 0 then
	server.sleep(1000,function()
	    statsprinter("name")
	    statsprinter("mastermode")
	end)
    end
end)

-- rewrite playercount, players, spectators at connect
server.event_handler("connect",function(cn)
    statsprinter("players")
    statsprinter("players","active")
    statsprinter("players","spec")
end)

-- rewrite playercount, players, spectators at disconnect
server.event_handler("disconnect",function(cn)
    statsprinter("players")
    statsprinter("players","active")
    statsprinter("players","spec")
end)

-- rewrite players, spectators at join/leave spectators
server.event_handler("spectator",function(cn,joined)
    statsprinter("players")
    statsprinter("players","active")
    statsprinter("players","spec")
end)

-- rewrite mastermode at mastermode-change
server.event_handler("setmastermode",function(old,new)
    statsprinter("mastermode",new)
end)

-- clear files at shutdown
server.event_handler("shutdown",function()
    local file = io.open("log/stats/name.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/host.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/mastermode.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/playercount.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/players.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/spectators.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/mode.html","w+")
    file:write("\n")
    file:flush()
    local file = io.open("log/stats/map.html","w+")
    file:write("\n")
    file:flush()
end)
