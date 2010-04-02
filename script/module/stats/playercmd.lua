
local backends = nil
local commandFunction = nil

local function initialize(tableOfBackends)
    
    backends = tableOfBackends
    
    server.playercmd_stats = commandFunction
end

commandFunction = function(cn, selection, subselection)

	local function currentgame_stats(sendto, player)

		local frags = server.player_frags(player) + server.player_suicides(player) + server.player_teamkills(player)
        
        local players = server.clients()
        local rank = -1
        table.sort(players, function(a, b) return server.player_frags(a) > server.player_frags(b) end)
        for i, cn in ipairs(players) do if cn == player then rank = i end end	

		server.player_msg(sendto, string.format("Current game stats for %s:", green(server.player_name(player))))
		server.player_msg(sendto, string.format("Score %s Frags %s Deaths %s Accuracy %s Rank %s",
			yellow(server.player_frags(player)),
			green(frags),
			red(server.player_deaths(player)),
			yellow(server.player_accuracy(player) .. "%"),
            blue(rank))
		)
		if gamemodeinfo.teams then
			server.player_msg(sendto,string.format("Teamkills: %s",red(server.player_teamkills(player))))
		end
	end
	
	local function total_stats(sendto, player)
    
	    if not backends.query then 
        	server.player_msg(sendto, red("Not available."))
        	return
        end
        
        row = backends.query.player_totals(server.player_name(cn))
        if not row then
            server.player_msg(sendto, "No stats found.")
            return
        end
        
        server.player_msg(sendto, string.format("Total game stats for %s:", green(server.player_name(player))))
        
        local kpd = round(row.frags / row.deaths, 2)
        local acc = round((row.hits / row.shots)*100)
        --TODO colour kpd and acc green or red depending on how good the values are relative to avg values
        
        server.player_msg(sendto, string.format("Games %s Frags %s Deaths %s Kpd %s Accuracy %s Wins %s Losses: %s",
        yellow(row.games),
        green(row.frags),
        red(row.deaths),
        yellow(kpd),
        yellow(acc .. "%"),
        green(row.wins),
        red(row.losses)))
	end
	
	if not selection then
	    currentgame_stats(cn, cn)
    elseif selection == "total" then
        total_stats(cn, cn)
    elseif selection == "all" then
        local players = server.clients()
        table.sort(players, function(a, b) return server.player_frags(a) > server.player_frags(b) end)
        for i, cn_ in ipairs(players) do 
            currentgame_stats(cn, cn_)
        end
    elseif server.valid_cn(selection) then
        if subselection == "total" then
            total_stats(cn, selection)
        else
            currentgame_stats(cn, selection)
        end
	else
        server.player_msg(cn, red("usage: #stats [cn] [total]"))
	end
end

return {initialize = initialize, command_function = commandFunction}
