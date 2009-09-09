-- #warning <cn> "<text>"

return function(warning_cn, warning_tcn, warning_text)

    if not warning_tcn then
    
        server.player_msg(warning_cn,red("#warning <cn> \"<text>\""))
        return
        
    elseif not warning_text then
    
        server.player_msg(warning_cn,red("text is missing"))
        
    elseif not server.valid_cn(warning_tcn) then
    
        server.player_msg(warning_cn,red("cn is not valid"))
        
    else
    
        local warns_count = (server.player_vars(warning_tcn).warnings_count or 1)
        
        if warns_count <= server.warnings_limit then
        
            server.player_msg(warning_tcn," ")
            local msg = "Warning"
            if warns_count == server.warnings_limit then
                msg = "Last Warning"
            end
            server.msg("(" .. red(msg) .. ")  " .. "(" .. green(server.player_name(warning_tcn)) .. ")  " .. orange(warning_text))
            server.player_msg(warning_tcn," ")
            server.player_vars(warning_tcn).warnings_count = warns_count + 1
        else
            server.kick(warning_tcn,server.warning_bantime,warning_cn,"")
            server.player_vars(warning_tcn).warnings_count = nil
        end
    end
end
