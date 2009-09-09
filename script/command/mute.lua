-- #mute <cn>

return function(mute_cn,mute_tcn)
    if not mute_tcn then
        server.player_msg(mute_cn,red("#mute <cn>"))
    elseif not server.valid_cn(mute_tcn) then
        server.player_msg(mute_cn,red("cn is not valid"))
    else
        server.mute(mute_tcn)
    end
end

