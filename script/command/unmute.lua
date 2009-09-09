
return function(unmute_cn, unmute_tcn)
    if not unmute_tcn then
        server.player_msg(unmute_cn,red("#unmute <cn>"))
    elseif not server.valid_cn(unmute_tcn) then
        server.player_msg(unmute_cn,red("cn is not valid"))
    else
        server.unmute(unmute_tcn)
    end
end
