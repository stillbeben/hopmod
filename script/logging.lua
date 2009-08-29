
local logfile = io.open("log/server.log","a+")

function log(msg)
    logfile:write(os.date("[%a %d %b %X] ",os.time()))
    logfile:write(msg)
    logfile:write("\n")
    logfile:flush()
end

server.log = log

function server.log_error(msg)
    io.stderr:write(os.date("[%a %d %b %X] ",os.time()))
    io.stderr:write(msg)
    io.stderr:write("\n")
    io.stderr:flush()
end

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
        
        log(string.format("Names used by %s(%i): %s", current_name, cn, namelist))
    end
    
end

server.event_handler("failedconnect", function(ip, reason)
    if reason == "normal" then reason = "client-side failure" end
    log(string.format("%s unable to connect: %s",ip, reason))
end)

server.event_handler("connect", function (cn)

    local ip = server.player_ip(cn)
    local country = server.ip_to_country(ip)
    
    log(string.format("%s(%i)(%s)(%s) connected",server.player_name(cn),cn,ip,country))

end)

server.event_handler("disconnect", function (cn,reason)
    
    local reason_tag = ""
    if reason ~= "normal" then reason_tag = " because: " .. reason end
    
    log(string.format("%s(%i) disconnected%s, time %s",server.player_name(cn),cn,reason_tag,server.format_duration(server.player_connection_time(cn))))
end)

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    log(string.format("%s(%i) was %s by %s %s",server.player_name(cn),cn,action_tag,admin,reason_tag))
end)

server.event_handler("rename",function(cn, oldname, newname)
    log(string.format("%s(%i) renamed to %s",oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    log(string.format("%s(%i) changed team to %s",server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)
    local mute_tag = ""
    if server.player_pvar(cn,"mute") then mute_tag = "(muted)" end
    log(string.format("%s(%i)%s: %s",server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    log(string.format("%s(%i)(team): %s",server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    log(string.format("%s(%i) suggests %s on map %s",server.player_name(cn),cn,mode,map))
end)

server.event_handler("mapchange", function(map, mode)
    
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = tostring(pc) .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    log(string.format("New game: %s on %s, %s", mode, map, playerstats))
end)

server.event_handler("setmastermode", function(oldmode, newmode)
    log(string.format("mastermode changed to %s",newmode))
end)

server.event_handler("setmaster", function(cn, priv, value)
    
    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end
    
    log(string.format("%s(%i) %s %s",server.player_name(cn),cn,action_tag,priv))
end)

server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    log(string.format("%s(%i) %s spectators",server.player_name(cn),cn,action_tag))
end)

server.event_handler("auth", function(cn, id, authname, authdomain, success)
    
    local action_tag = "passed"
    if tonumber(success) == 0 then action_tag = "failed" end
    
    if #authdomain == 0 then authdomain = server.masterserver end
    
    log(string.format("%s(%i) %s authentication as %s@%s",server.player_name(cn),cn,action_tag,authname,authdomain))
end)

server.event_handler("gamepaused", function() log("game is paused") end)
server.event_handler("gameresumed", function() log("game is resumed") end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = server.player_name(cn) .. string.format("(%i)", cn) end
    log(string.format("%s added a bot (skill %i)", addedby, skill))
end)

server.event_handler("delbot", function(cn)
    log(string.format("%s(%i) deleted a bot",server.player_name(cn),cn))
end)

server.event_handler("beginrecord", function(id,filename)
    log(string.format("recording game to %s",filename))
end)

server.event_handler("endrecord", function(id, size)
    log(string.format("finished recording game (%s file size)",format_filesize(tonumber(size))))
end)

server.event_handler("mapcrcfail", function(cn) 
    log(string.format("%s(%i) has a modified map (%s %i). [ip: %s]",server.player_name(cn),cn, server.map, server.player_mapcrc(cn), server.player_ip(cn)))
    log_usednames(cn)
end)

server.event_handler("shutdown", function() log("server shutting down"); logfile:close() end)

server.event_handler("reloadhopmod", function() log("reloading hopmod...") end)

log("server started")
