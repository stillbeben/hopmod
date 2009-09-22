dofile("./script/utils.lua")

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

local event_conditions = { disconnect = {}}

function event_conditions.disconnect.empty_server()
    return server.playercount == 0
end

function server.conditional_event_handler(event_name, condition_function, handler_function)
    
    if type(condition_function) == "string" then
        condition_function = event_conditions[event_name][condition_function]
        if not condition_function then
            error("Unknown condition function")
        end
    end
    
    local id = server.event_handler(event_name, function(...)
        if condition_function(unpack(arg)) then
            handler_function(unpack(arg))
        end
    end)
    
    return id
    
end


function server.group_players(arg1,arg2,arg3)

	if not arg1 then
		return -1
	end

	local tag = nil
	local team = nil

	if arg1 == "all" then
		if not arg2 then
			return -1
		end
		tag = arg2
		if arg3 then
			team = arg3
		else
			team = tag
		end

		for j,cn in ipairs(server.spectators()) do
			if string.find(server.player_name(cn),tag) then
				server.unspec(cn)
			end
		end
	else
		tag = arg1
		if arg2 then
			team = arg2
		else
			team = tag
		end
	end

	for p in server.gplayers() do
		if string.find(p:name(),tag) then
			server.changeteam(p.cn,team)
		end
	end

end
