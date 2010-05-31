dofile("./script/base/utils/string.lua")
dofile("./script/base/utils/table.lua")

-- Copied from http://lua-users.org/wiki/SimpleRound
function round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end
math.round = round

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

		for p in server.gclients() do

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

function pack(...)
    return arg
end

function identity(...)
    return unpack(arg)
end

function catch_error(fun, ...)

    local returnvals = pack(pcall(fun, unpack(arg)))
    
    if returnvals[1] == false then
        server.log_error(returnvals[2])
        return
    end
    
    table.remove(returnvals, 1)
    return unpack(returnvals)
end

function return_catch_error(fun, ...)

    local returnvals = pack(pcall(fun, unpack(arg)))
    
    if returnvals[1] == false then
        server.log_error(returnvals[2])
    end
    
    return unpack(returnvals)
end

do
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
end

function server.eval_lua(str)
    local func, err = loadstring(str)
    if not func then error(err) end
    return func()
end

function server.hashpassword(cn, pass)
	return crypto.tigersum(string.format("%i %i %s", cn, server.player_sessionid(cn), pass))
end

-- Array of game mode names, used by the map rotation module
gamemodes = {
    [ 1] = "ffa",
    [ 2] = "coop edit",
    [ 3] = "teamplay",
    [ 4] = "instagib",
    [ 5] = "instagib team",
    [ 6] = "efficiency",
    [ 7] = "efficiency team",
    [ 8] = "tactics",
    [ 9] = "tactics team",
    [10] = "capture",
    [11] = "regen capture",
    [12] = "ctf",
    [13] = "insta ctf",
    [14] = "protect",
    [15] = "insta protect",
    [16] = "hold",
    [17] = "insta hold",
    [18] = "efficiency ctf",
    [19] = "efficiency protect",
    [20] = "efficiency hold"
}

