-- #reload

-- [[ Player command written by Thomas

function server.playercmd_reload(cn_client)
    return admincmd(function () server.reloadscripts() end, cn_client)
end

-- ]]
