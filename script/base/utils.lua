
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

function list_to_set(list)
    local set = {}
    for _,value in ipairs(list) do set[value] = true end
    return set
end

function formatcol(col, text)
    if text then return "\fs\f" .. col .. text .. "\fr" else return "\f" ..col end
end

function red(text) return formatcol(3,text) end
function orange(text) return formatcol(6, text) end
function green(text) return formatcol(0, text) end
function white(text) return formatcol(9, text) end
function yellow(text) return formatcol(2, text) end
function magenta(text) return formatcol(5, text) end
function blue(text) return formatcol(1, text) end


local function createConsoleFunction()
    
    local triggerCallEvent = server.create_event_slot("admin-message")
    
    function server.console(admin, msg)
        server.msg(string.format("Remote Admin%s: %s", magenta("("..admin..")"), green(msg)))
        triggerCallEvent(admin, msg)
    end
end

createConsoleFunction()

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
	if #desc == 0 then desc = "<NONE>" end

	local mapname = server.map
	if #mapname == 0 then mapname = "<NONE>" end

	status_rows = status_rows .. string.format("%i/%i %s %s %i %s %i %s", server.playercount, server.maxclients, mapname, server.gamemode, mm, host, server.serverport, desc)

	out:write(tabulate(status_rows))
	out:write("\n")

	if server.playercount > 0 then

		local player_rows = "CN LAG PING IP CO NAME TIME STATE PRIV\n"

		for p in server.aplayers() do

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

function server.teamsize(teamsize_teamname)
    local a = 0
    for i,cn in ipairs(server.team_players(teamsize_teamname)) do
        a = a + 1
    end
    return a
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

function validate_table(subject_table, schema)
    
    local function check_type(object, typename)
        if type(object) ~= typename then error("expecting " .. typename) end
    end
    
    check_type(subject_table, "table")
    
    for _, element in ipairs(schema) do
        
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

function print_list(...)
    local output = ""
    for _, item in ipairs(arg) do
        item = tostring(item)
        if #item > 0 then
            if #output > 0 then output = output .. ", " end
            output = output .. item
        end
    end
    return output
end

local alias_of_mode = {
    ["instagib team"]    = "instagib team",
    ["instateam"]        = "instagib team",
    ["iteam"]            = "instagib team",
    ["instagib"]         = "instagib",
    ["insta"]            = "instagib",
    ["insta ctf"]        = "insta ctf",
    ["instactf"]         = "insta ctf",
    ["ictf"]             = "insta ctf",
    ["ctf"]              = "ctf",
    ["insta protect"]    = "insta protect",
    ["instaprotect"]     = "insta protect",
    ["iprotect"]         = "insta protect",
    ["protect"]          = "protect",
    ["teamplay"]         = "teamplay",
    ["ffa"]              = "ffa",
    ["efficiency team"]  = "efficiency team",
    ["efficteam"]        = "efficiency team",
    ["eteam"]            = "efficiency team",
    ["efficiency"]       = "efficiency",
    ["effic"]            = "efficiency",
    ["tactics team"]     = "tactics team",
    ["tacteam"]          = "tactics team",
    ["tteam"]            = "tactics team",
    ["tactics"]          = "tactics",
    ["tac"]              = "tactics",
    ["regen capture"]    = "regen capture",
    ["regencapture"]     = "regen capture",
    ["regencap"]         = "regen capture",
    ["regen"]            = "regen capture",
    ["capture"]          = "capture",
    ["cap"]              = "capture",
    ["coop edit"]        = "coop edit",
    ["coopedit"]         = "coop edit",
    ["coop"]             = "coop edit"
}

function server.parse_mode(mode)
    if mode then
        return alias_of_mode[mode]
    end
    return nil
end

function server.eval_lua(str)
    local func, err = loadstring(str)
    if not func then error(err) end
    return func()
end
