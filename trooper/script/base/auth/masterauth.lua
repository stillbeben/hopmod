
auth.directory.server{
    id = "MASTER",
    hostname = "sauerbraten.org",
    port = 28787
}

auth.directory.domain{
    server = "MASTER",
    id = ""
}

local banned = list_to_set(table_unique(server.parse_list(server.masterauth_banned)))

auth.listener("", function(cn, user_id, domain, status)
    
    if status ~= auth.request_status.SUCCESS then return end
    
    local admin_present = server.master ~= -1 and server.player_priv_code(server.master) == server.PRIV_ADMIN
    
    if server.player_priv_code(cn) == 0 and not admin_present and server.use_master_auth == 1 then
        
        local name = server.player_name(cn)
        
        if banned[user_id] then
            server.player_msg(cn, red("You have been banned from using /auth on this server"))
            return
        end
        
        if server.setmaster(cn) then
            server.msg(string.format("%s claimed master as '%s'", name, magenta(user_id)))
            server.log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
        else
            server.player_msg(cn, red("Master is disabled."))
        end
    end

end)
