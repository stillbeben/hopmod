function server.playercmd_unmute(unmute_cn,unmute_tcn)
    return mastercmd(function()
        if not unmute_tcn then
            server.player_msg(unmute_cn,red("#unmute <cn>"))
        elseif server.valid_cn(umute_tcn) then
            server.player_msg(unmute_cn,red("cn is not valid"))
        else
            server.unmute(unmute_tcn)
        end
    end,unmute_cn)
end
