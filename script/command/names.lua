-- #names <cn>

function server.playercmd_names(cn, target_cn)
    if server.valid_cn(target_cn) then
	local db = sqlite3.open(server.stats_db_filename)
        local str = "Other names used by " .. server.player_name(target_cn) .. ": "
        for name, count in db:cols("SELECT DISTINCT name, count(name) as count FROM players WHERE ipaddr = '" .. server.player_ip(target_cn) .. "'") do
            str = str .. name .. "(" .. count .. "),"
        end
        server.player_msg(cn, str)
        db:close(db)
    else
        server.player_msg(cn,red("cn is not valid"))
    end
end
