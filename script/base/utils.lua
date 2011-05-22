function mins(value)
    return value * 1000 * 60
end

function secs(value)
    return value * 1000
end

function concol(colour_code, text)
    return "\fs\f" .. colour_code .. text .. "\fr"
end

function bind_concol(colour_code)
    return function(text)
        return concol(colour_code, text)
    end
end

green    = bind_concol(0)
info     = bind_concol(1)
err      = bind_concol(3)
grey     = bind_concol(4)
magenta  = bind_concol(5)
orange   = bind_concol(6)
gameplay = bind_concol(2)
red      = bind_concol(3)
blue     = bind_concol(1)
yellow   = bind_concol(2)

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

function catch_error(chunk, ...)
    
    local pcall_results = pack(pcall(chunk, unpack(arg)))
    
    if not pcall_results[0] then
        server.log_error(pcall_results[1])
    end
    
    return pcall_results
end

function server.eval_lua(str)
    local func, err = loadstring(str)
    if not func then error(err) end
    return func()
end

function server.hashpassword(cn, password)
    return crypto.tigersum(string.format("%i %i %s", cn, server.player_sessionid(cn), password))
end

function _if(expr, true_value, false_value)
    if expr then
        return true_value
    else
        return false_value
    end
end

dofile("./script/base/utils/apps.lua")
dofile("./script/base/utils/file.lua")
dofile("./script/base/utils/gamemode.lua")
dofile("./script/base/utils/mysql.lua")
dofile("./script/base/utils/string.lua")
dofile("./script/base/utils/table.lua")
dofile("./script/base/utils/validate.lua")
dofile("./script/base/utils/deferred.lua")
dofile("./script/base/utils/event_emitter.lua")

