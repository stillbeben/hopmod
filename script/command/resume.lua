-- #resume

function server.playercmd_resume(cn)
    return admincmd(function ()
        server.pausegame(false)
    end, cn)
end
