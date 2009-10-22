
function server.deleteFile(filename)
    local file = io.open(filename, "w+")
    file:close()
    os.remove(filename)
end

function server.fileExists(filename)
    local file = io.open(filename, "r")
    if file ~= nil then return true else return false end
end

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

function server.specall()
    for i,cn in ipairs(server.players()) do server.spec(cn) end
end

function server.unspecall()
    for i,cn in ipairs(server.spectators()) do server.unspec(cn) end
end

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

function server.printserverstatus(filename, filemode)

    if not filemode then
        filemode = "a+"
    end
    
    local out = io.open(filename, filemode)
    
    local status_rows = "PLAYERS MAP MODE MASTER HOST PORT DESCRIPTION\n"
    local host = server.serverip
    if #host == 0 then host="<ANY>" end
    local mm = server.mastermode
    local desc = string.gsub(server.servername, " ", "_")
    if #desc == 0 then desc = "<EMPTY>" end
    
    status_rows = status_rows .. string.format("%i/%i %s %s %i %s %i %s", server.playercount, server.maxplayers, server.map, server.gamemode, mm, host, server.serverport, desc)
    
    out:write(tabulate(status_rows))
    out:write("\n")
    

    if server.playercount > 0 then
    
        local player_rows = "CN LAG PING IP CO NAME TIME STATE PRIV\n"
        
        for p in server.gplayers("all") do
            
            local country = geoip.ip_to_country_code(p:ip())
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

function server.name_to_cn(name)

    if not name then
        return
    end
    
	name = string.lower(name)

    local name_matches = 0
    local first_matching_cn
    
    for i,cn in pairs(server.players()) do
    
        if string.lower(server.player_name(cn)) == name then
        
            first_matching_cn = first_matching_cn or cn
            name_matches = name_matches + 1
            
        end
        
    end
    
    if name_matches == 1 then
    
        return first_matching_cn
        
    elseif name_matches == 0 then
    
        local substring_matches = {}
        
        for i,cn in pairs(server.players()) do
            
            local candidate = server.player_name(cn)
            
            if string.find(string.lower(candidate), name) then
                table.insert(substring_matches, {name=candidate,cn=cn})
            end
        end
        
        return nil, substring_matches
        
    elseif name_matches > 1 then
        return nil, name_matches
    end
    
end

function server.disambiguate_name_list(cn, name)

	local message = ""

	name = string.lower(name)

	for p in server.gplayers() do
		local pname = p:name()

		if name == string.lower(pname) then
			message = message .. string.format("%i %s\n", p.cn, pname)
		end
	end

	server.player_msg(cn, message)

end

function server.similar_name_list(cn, names)

	local message = ""

	for i, player in pairs(names) do
		message = message .. string.format("%i %s\n", player.cn, player.name)
	end

	server.player_msg(cn, message)

end

function server.name_to_cn_list_matches(cn,name)

	local lcn, info = server.name_to_cn(name)

	if not lcn then

		if type(info) == "number" then  -- Multiple name matches

			server.player_msg(cn, red(string.format("There are %i players here matching that name:", info)))
			server.disambiguate_name_list(cn,name)

			return nil

		elseif #info == 0 then  -- no matches

			server.player_msg(cn, red("There are no players found matching that name."))

			return nil

		else    -- Similar matches

			server.player_msg(cn, red("There are no players found matching that name, but here are some similar names:"))
			server.similar_name_list(cn, info)

			return nil
		end

		return nil

	else

		return lcn
	end

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

function pack(...)
    return arg
end

function catch_error(fun, ...)

    local returnvals = pack(pcall(fun, unpack(arg)))
    
    if returnvals[1] == false then
        server.log_error(returnvals[2])
        return
    end
    
    if returnvals then
    
        table.remove(returnvals, 1)
    
        return unpack(returnvals)
    end
    
end


function server.all_players()

	local list = server.players()

	for i,c in ipairs(server.spectators()) do
        table.insert(list,c)
	end

	return list
end

function validate_table(subject_table, schema)
    
    local function check_type(object, typename)
        if type(object) ~= typename then error("expecting " .. typename) end
    end
    
    check_type(subject_table, "table")
    
    for i, element in ipairs(schema) do
        
        local id = element[1]
        local typeinfo = element[2]
        
        if not id then error("error in table schema") end
        
        local lookup = subject_table[id]
        
        if lookup == nil then
            error("missing " .. id)
        end
        
        if typeinfo then
            
            if type(typeinfo) == "string" then
                
                check_type(lookup, typeinfo)
                
            elseif type(typeinfo) == "table" then
                
                validate_table(lookup, typeinfo)
                
            else
                error("error in table schema")
            end
            
        end
        
    end
    
end

-- Copied from the lua-users wiki
function table.deepcopy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

function server.is_teamkill(player1, player2)
    if not gamemodeinfo.teams then return false end
    if server.player_team(player1) == server.player_team(player2) then return true end
    return false
end
