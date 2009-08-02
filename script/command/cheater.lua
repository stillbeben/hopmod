-- #cheater <cn>|"<name>"

server.event_handler("started",function()
    if server.cheater_ad_timer ~= 0 then
	server.interval(server.cheater_ad_timer,function()
	    if (tonumber(server.playercount) > 2) and (tonumber(server.mastermode) == 0) then
		server.msg("(" .. green("Info") .. ")  " .. "If you suspect a cheater type " .. yellow("#cheater \"<name>\"") .. " or " .. yellow("#cheater <cn>") .. " to notify an admin.")
	    end
	end)
    end
end)

function server.playercmd_cheater(cn,cheat)
    if not cheat then
	server.player_msg(cn,red("#cheater (<cn>|\"<name>\")"))
    end
    if not server.valid_cn(cheat) then
	cheat = server.find_cn(cn,cheat)
	if not cheat then
	    return
	end
    end
    cheat = tonumber(cheat)
    if server.player_sessionid(cheat) ~= -1 then
	local cheat_report = server.player_pvars(cn).cheat_report
	if not cheat_report then
	    cheat_report = 0
	end
	cheat_report = cheat_report + 1
        server.player_pvars(cn).cheat_report = cheat_report
        if cheat_report > 4 then
            server.player_msg(cn,"Don't spam with the #cheater command or you will be ignored.")
        end
        if cheat_report < 8 then
            if server.player_connection_time(cheat) > 3 then
                server.player_msg(cn,"Thank you for your report, hopefully an admin will check this out very soon.")
                server.log("CHEATER: " .. server.player_name(cheat) .. " was reported by " .. server.player_name(cn))
            end
        end
    end
end
