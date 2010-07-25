-- Copied from http://lua-users.org/wiki/SimpleRound
function math.round(num, idp)
  local mult = 10^(idp or 0)
  return math.floor(num * mult + 0.5) / mult
end
round = math.round

function pack(...)
    return arg
end

function identity(...)
    return unpack(arg)
end

function return_catch_error(fun, ...)

    local returnvals = pack(pcall(fun, unpack(arg)))
        
    if returnvals[1] == false and returnvals[2] then
        server.log_error(returnvals[2])
    end
    
    return unpack(returnvals, 1, table.maxn(returnvals))
end

function catch_error(fun, ...)
    local returnvals = pack(return_catch_error(fun, unpack(arg)))
    table.remove(returnvals, 1)
    return unpack(returnvals, 1, table.maxn(returnvals))
end

function server.eval_lua(str)
    local func, err = loadstring(str)
    if not func then error(err) end
    return func()
end

function server.hashpassword(cn, pass)
	return crypto.tigersum(string.format("%i %i %s", cn, server.player_sessionid(cn), pass))
end

dofile("./script/base/utils/string.lua")
dofile("./script/base/utils/table.lua")
dofile("./script/base/utils/gamemode.lua")
dofile("./script/base/utils/apps.lua")

