
function admincmd(...)
    local func = arg[1]
    local cn = arg[2]
    
    if tonumber(server.player_priv_code(cn)) < tonumber(server.PRIV_ADMIN) then
        server.player_msg(cn,red("Permission denied."))
        return
    end
    
    table.remove(arg,1)
    
    return func(unpack(arg))
end

function server.playercmd_info(cn)
    server.player_msg(cn, orange(server.servername) .. ": " .. server.motd )
end

function server.playercmd_specall(cn)
    return admincmd(server.specall,cn)
end

function server.playercmd_unspecall(cn)
    return admincmd(server.unspecall,cn)
end

function server.playercmd_players(cn_client) 
    for i, cn in ipairs(server.players()) do 
        str = "Name: " .. server.player_name(cn) .. " Frags: " .. server.player_frags(cn) .. " Deaths: " .. server.player_deaths(cn) .. " Acc: " .. server.player_accuracy (cn)
        server.player_msg(cn_client, str)
    end
end
