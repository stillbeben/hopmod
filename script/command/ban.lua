-- #ban [all] <cn> [<time> [m|h]] ["<reason>"]

return function(ban_cn,arg1,arg2,arg3,arg4,arg5)

    local all = 0
    local cn = nil
    local time = 3600
    local reason = ""
    if not arg1 then
        server.player_msg(ban_cn,red("#ban [all] <cn> [<time> [m|h]] [\"<reason>\"]"))
        return
    elseif arg1 == "all" then
        all = 1
        if not arg2 then
            server.player_msg(ban_cn,red("cn is missing"))
            return
        elseif server.valid_cn(arg2) then
            cn = arg2
    else
            server.player_msg(ban_cn,red("cn is not valid"))
            return
        end
    if arg3 then
        if arg3 > "0" and arg3 < "13670" then
                time = tonumber(arg3)
                if arg4 then
                    if arg4 == "h" then
                        time = time * 3600
                    elseif arg4 == "m" then
                        time = time * 60
                    elseif arg5 then
                        server.player_msg(ban_cn,red("time.unit is not valid"))
                        return
        else
                        reason = arg4
                    end
                    if arg5 then
                        reason = arg5
                    end
                end
    else
        reason = arg3
    end
    end
    elseif server.valid_cn(arg1) then
    cn = arg1
        if arg2 then
        local larg2 = tonumber(arg2)
            if larg2 > 0 and larg2 < 13670 then
                time = larg2
                if arg3 then
                    if arg3 == "h" then
                        time = time * 3600
                    elseif arg3 == "m" then
                        time = time * 60
                    elseif arg4 then
                        server.player_msg(ban_cn,red("time.unit is not valid"))
                        return
                    else
            reason = arg3
                    end
                    if arg4 then
                        reason = arg4
                    end
        end
    else
                reason = arg2
            end
        end
    else
        server.player_msg(ban_cn,red("cn is not valid"))
        return
    end
    if all == 1 then
        server.kick_bannedip_group = 1
    end
    server.kick(cn,time,server.player_name(ban_cn),reason)
    if all == 1 then
        server.kick_bannedip_group = 0
    end

end
