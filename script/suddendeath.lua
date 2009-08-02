local function sdmode(sdmode_cn,sdmode_option)
    player_score = {}
    sudden_death = "false"
    if sdmode_option == 1 then
        sudden_death_enabled = "true"
        server.msg(orange("--[ Sudden Death Mode Enabled. There will be no ties"))
    else
        sudden_death_enabled = "false"
        server.msg(orange("--[ Sudden Death Mode Disabled. There may be ties"))
    end
end

server.event_handler("timeupdate", function (mins)
    if sudden_death_enabled == "true" then
        if mins == 1 then
            server.sleep(59000, function()
                for index, cn in ipairs(server.players()) do
                    player_score[index] = server.player_frags(cn)
                end
                if player_score[1] == player_score[2] then
                    server.msg(red("--[ Sudden Death. Next Frag Wins!"))
                    sudden_death = "true"
                end
                return 1
            end)
	end
        if sudden_death == "true" then
	    return 1
	else
	    return mins
	end
    else
	return mins
    end
end)

server.event_handler("frag", function (target,actor)
    if sudden_death_enabled == "true" then
	if sudden_death == "true" then
            sudden_death = "false"
            server.changetime(0)
        end
    end
end)

server.event_handler("mapchange", function (map, mode)
    sudden_death = "false"
end)

-- #nosd
if server.enable_nosd_command == 1 then
    if server.enable_sd_command == 0 then
        server.log("WARNING: #nosd available, but not #sd")
    end
    function server.playercmd_nosd(nosd_cn)
	return admincmd(function()
            sdmode(nosd_cn,0)
        end,nosd_cn)
    end
end

-- #sd
if server.enable_sd_command == 1 then
    if server.enable_nosd_command == 0 then
        server.log("WARNING: #sd available, but not #nosd")
    end
    function server.playercmd_sd(sd_cn)
	return admincmd(function()
            sdmode(sd_cn,1)
        end,sd_cn)
    end
end
