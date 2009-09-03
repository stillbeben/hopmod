dofile("./script/utils.lua")
dofile("./script/playervars.lua")

-- Copied from http://lua-users.org/wiki/SimpleRound
function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end

-- Copied from http://lua-users.org/wiki/TableUtils
-- Count the number of times a value occurs in a table 
function table_count(tt, item)
  local count
  count = 0
  for ii,xx in pairs(tt) do
    if item == xx then count = count + 1 end
  end
  return count
end

-- Copied from http://lua-users.org/wiki/TableUtils
-- Remove duplicates from a table array (doesn't currently work
-- on key-value tables)
function table_unique(tt)
  local newtable
  newtable = {}
  for ii,xx in ipairs(tt) do
    if(table_count(newtable, xx) == 0) then
      newtable[#newtable+1] = xx
    end
  end
  return newtable
end

function format_filesize(bytes)
    bytes = tonumber(bytes)
    if bytes < 1024 then return bytes .. "B"
    elseif bytes < (1024*1024) then return round(bytes/1024) .. "KB"
    else return round(bytes/(1024*1024)) .. "MB"
    end
end

function format_duration(seconds)
    local hours = math.floor(seconds / 3600)
    seconds = seconds - (hours * 3600)
    local mins = math.floor(seconds / 60)
    seconds = seconds - (mins * 60)
    return string.format("%02i:%02i:%02i",hours,mins,seconds)
end

server.format_duration = format_duration

function map_to_array(map)
    local result = {}
    for i,v in pairs(map) do
        table.insert(result, v)
    end
    return result
end

function formatcol(col, text)
    if text then return "\fs\f" .. col .. text .. "\fr" else return "\f" ..col end
end

function red(text) return formatcol(3,text) end
function orange(text) return formatcol(6, text) end
function green(text) return formatcol(0, text) end
function white(text) return formatcol(10, text) end
function yellow(text) return formatcol(2, text) end
function magenta(text) return formatcol(5, text) end
function blue(text) return formatcol(1, text) end

local set_cancel_restart = false

function server.restart()
    
    if tonumber(server.playercount) == 0 then return server.restart_now() end

    set_cancel_restart = false
    local warning_msg = "The server is set for a restart at the end of this game."
    
    server.msg(warning_msg)
    
    local con_handler -- seems it must be declared before assignment to be included in event handler's closure
    con_handler = server.event_handler("connect", function(cn)
    
        if set_cancel_restart then
            server.cancel_handler(con_handler)
        else
            server.player_msg(cn, warning_msg) 
        end
    end)
    
    local mapch_handler
    mapch_handler = server.event_handler("setnextgame", function()
        
        if set_cancel_restart then
           server.cancel_handler(mapch_handler)
        else
            server.restart_now()
        end
    end)
    
end

function server.cancel_restart()
    server.msg("Server restart cancelled.")
    set_cancel_restart = true
end

function server.log_status(msg)
    print(msg)
end

function server.mute(cn)
    server.player_pvar(cn, "mute", true)
end

function server.unmute(cn)
    server.player_unsetpvar(cn, "mute")
end

function server.specall()
    for i,cn in ipairs(server.players()) do server.spec(cn) end
end

function server.unspecall()
    for i,cn in ipairs(server.spectators()) do server.unspec(cn) end
end

function update_gamemodeinfo()
    gamemodeinfo = server.gengamemodeinfo()
    server.gamemodeinfo = gamemodeinfo
end
server.event_handler("mapchange", update_gamemodeinfo)
update_gamemodeinfo()

function server.console(admin,msg)
    server.msg(string.format("Remote Admin%s: %s", magenta("("..admin..")"), green(msg)))
end

server.system = os.execute

function string.split(s, pattern)
    local a = {}
    for x in string.gmatch(s, pattern) do
        a[#a+1] = x
    end
    return a
end

function tabulate(text)
    
    local output = ""
    local cols = {}
    local rows = text:split("[^\n]+")
    
    for i in ipairs(rows) do
        
        rows[i] = string.split(rows[i], "[%w%p/*]+")
        
        for i,col in ipairs(rows[i]) do
            cols[i] = math.max(#col, cols[i] or 0)
        end
    end
    
    for i,row in ipairs(rows) do
    
        for i2, col in ipairs(row) do
            output = output .. col .. string.rep(" ",(cols[i2] - #col)+1)
        end
        
        output = output .. "\n"
    end
    
    return output
end

function server.new_player_object(cn)
    return {
        cn = cn,
        msg = function(obj, text) return server.player_msg(obj.cn, text) end,
        kick = function(obj) return server.kick(obj.cn) end,
        name = function(obj) return server.player_name(obj.cn) end,
        team = function(obj) return server.player_team(obj.cn) end,
        priv = function(obj) return server.player_priv(obj.cn) end,
        priv_code = function(obj) return server.player_priv_code(obj.cn) end,
        id = function(obj) return server.player_id(obj.cn) end,
        sessionid = function(obj) return server.player_sessionid(obj.cn) end,
        ping = function(obj) return server.player_ping(obj.cn) end,
        lag = function(obj) return server.player_lag(obj.cn) end,
        ip = function(obj) return server.player_ip(obj.cn) end,
        iplong = function(obj) return server.player_iplong(obj.cn) end,
        status = function(obj) return server.player_status(obj.cn) end,
        status_code = function(obj) return server.player_status_code(obj.cn) end,
        frags = function(obj) return server.player_frags(obj.cn) end,
        deaths = function(obj) return server.player_deaths(obj.cn) end,
        suicides = function(obj) return server.player_suicides(obj.cn) end,
        teamkills = function(obj) return server.player_teamkills(obj.cn) end,
        damage = function(obj) return server.player_damage(obj.cn) end,
        damagewasted = function(obj) return server.player_damagewasted(obj.cn) end,
        maxhealth = function(obj) return server.player_maxhealth(obj.cn) end,
        health = function(obj) return server.player_health(obj.cn) end,
        gun = function(obj) return server.player_gun(obj.cn) end,
        hits = function(obj) return server.player_hits(obj.cn) end,
        shots = function(obj) return server.player_shots(obj.cn) end,
        accuracy = function(obj) return server.player_accuracy(obj.cn) end,
        timeplayed = function(obj) return server.player_timeplayed(obj.cn) end,
        win = function(obj) return server.player_win(obj.cn) end,
        slay = function(obj) return server.player_slay(obj.cn) end,
        changeteam = function(obj,newteam) return server.player_changeteam(obj.cn,newteam) end,
        bots = function(obj) return server.player_bots(obj.cn) end,
        authreq = function(obj) return server.player_authreq(obj.cn) end,
        rank = function(obj) return server.player_rank(obj.cn) end,
        isbot = function(obj) return returnserver.player_isbot(obj.cn) end,
        mapcrc = function(obj) return server.player_mapcrc(obj.cn) end,
        connection_time = function(obj) return server.player_connection_time(obj.cn) end,
        spec = function(obj) return server.player_spec(obj.cn) end,
        unspec = function(obj) return server.player_unspec(obj.cn) end,
        setadmin = function(obj) return server.player_setadmin(obj.cn) end,
        setmaster = function(obj) return server.player_setmaster(obj.cn) end,
        set_invadmin = function(obj) return server.player_set_invadmin(obj.cn) end,
        unset_invadmin = function(obj) return server.player_unset_invadmin(obj.cn) end,
        vars = function(obj) return server.player_vars(obj.cn) end,
        pvars = function(obj) return server.player_pvars(obj.cn) end,
        var = function(obj, name, value, defvalue) return server.player_var(obj.cn, name, value, defvalue) end,
        pvar = function(obj, name, value, defvalue) return server.player_pvar(obj.cn, name, value, defvalue) end,
        hasvar = function(obj, name) return server.player_hasvar(obj.cn, name) end,
        haspvar = function(obj, name) return server.player_haspvar(obj.cn, name) end,
        pos = function(obj) return server.player_pos(obj.cn) end
    }
end

function server.gplayers()

    local function next_player(plist, player)
       
        local index = nil
        if player then index = player.index end
        
        local key, cn = next(plist, index)
        if not key then return nil end
        
        local nplayer = server.new_player_object(cn)
        nplayer.index = key
        
        return nplayer
    end
    
    local pv = server.players()
    return next_player, pv, nil
end

function server.printserverstatus(filename,filemode)
    if not filemode then
	filemode = "a+"
    end
    
    local out = io.open(filename, filemode)
    
    local status_rows = "PLAYERS MAP MODE MASTER HOST PORT DESCRIPTION\n"
    local host = server.serverip
    if #host == 0 then host="<ANY>" end
    local mm = server.mastermode
    local desc = server.servername -- TODO replace whitespaces with underscores
    if #desc == 0 then desc = "<EMPTY>" end
    
    status_rows = status_rows .. string.format("%i/%i %s %s %i %s %i %s", server.playercount, server.maxplayers, server.map, server.gamemode, mm, host, server.serverport, desc)
    
    out:write(tabulate(status_rows))
    out:write("\n")
    
    if server.playercount > 0 then
    
        local player_rows = "CN LAG PING IP CO NAME TIME STATE PRIV\n"
        
        for p in server.gplayers() do
            
            local country = server.ip_to_country_code(p:ip())
            if #country == 0 then country = "?" end
            
            local priv = p:priv()
            if server.master == cn then priv = "*" .. priv end
            
            local player_row = string.format("%i %i %i %s %s %s %s %s %s",
               p.cn, p:lag(), p:ping(), p:ip(), country, p:name(), format_duration(p:connection_time()), p:status(), priv)
            
            player_rows = player_rows .. player_row .. "\n"
            
        end
        
        out:write("\n")
        out:write(tabulate(player_rows))
        
    end
    
    out:flush()
    out:close()
end

function server.valid_cn(cn)
    return server.player_id(tonumber(cn) or -1) ~= -1
end

function server.teamsize(teamsize_teamname)
    local a = 0
    for i,cn in ipairs(server.team_players(teamsize_teamname)) do
        a = a + 1
    end
    return a
end

function server.find_cn(cn,name)
    if not name then
	return -1
    end
    local tcn = nil
    local tcn_count = 0
    for a,b in ipairs(server.players()) do
	if server.player_name(b) == name then
	    tcn = b
	    tcn_count = tcn_count + 1
	end
    end
    if tcn_count == 1 then
	return tcn
    elseif tcn_count == 0 then
	server.player_msg(cn,red("no player found"))
	local count = 0
	for a,b in ipairs(server.players()) do
	    local pname = server.player_name(b)
	    if string.find(pname,name) then
		if count == 0 then
		    server.player_msg(cn,orange("matching players are:"))
		end
		server.player_msg(cn,green(pname) .. " with cn: " .. green(b))
		count = count + 1
	    end
	end
    elseif tcn_count > 1 then
	server.player_msg(cn,red("more than one player found"))
	for a,b in ipairs(server.players()) do
	    local pname = server.player_name(b)
	    if pname == name then
		server.player_msg(cn,green(pname) .. " has cn: " .. green(b))
	    end
	end
    end
    return
end

local function random_init()
    local rand_non_null_value = (tonumber(server.serverport) * (tonumber(server.gamelimit) + tonumber(server.maxplayers))) + tonumber(server.serverport)
    math.randomseed(math.abs(((os.time() * (os.clock() * 10000)) / rand_non_null_value) + (os.time() * os.time())))
end
random_init()
server.sleep((math.random(60,1800) * 1000),function()
    random_init()
    server.sleep((math.random(60,1800) * 1000),function()
      random_init()
    end)
end)

function server.random_map(random_map_mode,random_map_small)
    if not random_map_small then
        random_map_small = 1
    end
    local random_map_list = {}
    local parse = server.parse_list
    if random_map_small == 1 and ( random_map_mode == "ffa" or random_map_mode == "teamplay" or random_map_mode == "efficiency" or random_map_mode == "efficiency team" or random_map_mode == "tactics" or random_map_mode == "tactics team" or random_map_mode == "instagib" or random_map_mode == "instagib team" ) then
        random_map_list = table_unique(parse(server["small_" .. random_map_mode .. "_maps"]))
    else
        random_map_list = table_unique(parse(server[random_map_mode .. "_maps"]))
    end
    if not random_map_list then
        return nil
    end
    
    return random_map_list[math.random(#random_map_list)]
end

function server.random_mode()
    local random_mode_list = table_unique(server.parse_list(server["game_modes"]))
    if not random_mode_list then
        return nil
    end
    
    return random_mode_list[math.random(#random_mode_list)]
end
