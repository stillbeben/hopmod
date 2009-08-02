function server.playercmd_mute(mute_cn,mute_tcn)
    return mastercmd(function()
        if not mute_tcn then
            server.player_msg(mute_cn,red("#mute <cn>"))
        elseif server.valid_cn(mute_tcn) then
            server.player_msg(mute_cn,red("cn is not valid"))
        else
            server.mute(mute_tcn)
        end
    end,mute_cn)
end
