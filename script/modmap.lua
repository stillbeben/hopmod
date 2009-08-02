local function modmap_stresser(modmap_stresser_cn)
    server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(modmap_stresser_cn)) .. " has a " .. red("modified map"))
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  You have a " .. red("modified map"))
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  " .. orange("You shouldn't play on this map!") .. " Please, wait for the next map")
    server.player_msg(modmap_stresser_cn,"(" .. green("Info") .. ")  and " .. orange("re-download the game") .. " from " .. yellow("www.sauerbraten.org"))
    server.spec(modmap_stresser_cn)
end

server.event_handler("mapcrcfail",function(modmap_mapcrcfail_cn)
    if (tonumber(server.playercount) >= 3) and ( not (server.gamemode == "coop edit") ) and ( server.is_known_map(server.map) ) then
	modmap_stresser(modmap_mapcrcfail_cn)
    end
end)
