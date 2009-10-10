
auth.directory.server{
    id = "MASTER",
    hostname = "sauerbraten.org",
    port = 28787
}

auth.directory.domain{
    server = "MASTER",
    id = ""
}

auth.listener("", function(cn, user_id, domain, status)
    
    if status ~= auth.request_status.SUCCESS then return end
    
    local admin_present = server.master ~= -1 and server.player_priv_code(server.master) == server.PRIV_ADMIN
    
    if server.player_priv_code(cn) == 0 and not admin_present and server.using_master_auth() then
        
        if server.setmaster(cn) then
            server.msg(string.format("%s claimed master as '%s'",server.player_name(cn),magenta(user_id)))
        else
            server.player_msg(cn, red("Master is disabled."))
        end
    end

end)
