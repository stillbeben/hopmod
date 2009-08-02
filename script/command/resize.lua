-- #resize <number>

function server.playercmd_resize(resize_cn,resize_size)
    return admincmd(function()
        if not resize_size then
            server.maxplayers = resize_normalmaxplayers
        elseif tonumber(resize_size) < 0 then
            server.player_msg(resize_cn,red("size is not an allowed number"))
            return
        else
            if tonumber(resize_size) < server.resize_totalminplayers then
                resize_size = server.resize_totalminplayers
            elseif tonumber(resize_size) > server.resize_totalmaxplayers then
                resize_size = server.resize_totalmaxplayers
            end
            server.maxplayers = resize_size
        end
    end,resize_cn)
end

server.event_handler("disconnect",function(cn)
    if tonumber(server.playercount) == 0 then
        server.maxplayers = resize_normalmaxplayers
    end
end)

server.event_handler("started",function()
    resize_normalmaxplayers = server.maxplayers
end)
