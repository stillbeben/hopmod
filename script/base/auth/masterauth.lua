
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

auth.listener("", function(cn, user_id, domain, status, kick_cn)

    if status ~= auth.request_status.SUCCESS then return end

    local priv_code = server.player_priv_code(cn)

    if banned[user_id] then
        if priv_code == 0 then
            server.player_msg(cn, red("You have been banned from using /auth on this server"))
        end
        return
    end

    local name = server.player_name(cn)

    if kick_cn ~= nil and kick_cn >= 0 then
        if not server.valid_cn(kick_cn) then
            server.player_msg(cn, red("Invalid player cn given"))
            return
        end

        if server.player_priv_code(kick_cn) > server.PRIV_AUTH then
            server.player_msg(cn, red("Kick player request denied"))
            return
        end

        local vitcim = server.player_displayname(kick_cn)
        server.kick(kick_cn, nil, name, string.format("%s as '%s' kicked %s", server.player_displayname(cn), magenta(user_id), vitcim))

        return
    end

    if priv_code == 0 then
        if server.setauth(cn) then
            server.msg(string.format("%s claimed master as '%s'", server.player_displayname(cn), magenta(user_id)))
            server.log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
        end
    end
end)
