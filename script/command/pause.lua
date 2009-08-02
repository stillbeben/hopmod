function server.playercmd_pause(cn)
    return admincmd(function ()
        server.pausegame(true)
    end, cn)
end
