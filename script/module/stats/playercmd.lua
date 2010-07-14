
local backends = nil

local function initialize(tableOfBackends)
    backends = tableOfBackends
end

local function total_stats(sendto, player)
    
    if not backends.query then 
        return false, "service unavailable, try again later"
    end
    
    row = backends.query.player_totals(server.player_name(player))
    if not row then
        server.player_msg(sendto, "No stats found.")
        return
    end
    
    if sendto ~= player then
        server.player_msg(sendto, string.format("Total game stats for %s:", green(server.player_name(player))))
    end
    
    local kpd = round(row.frags / row.deaths, 2)
    local acc = round((row.hits / row.shots)*100)
    --TODO colour kpd and acc green or red depending on how good the values are relative to avg values
    
    server.player_msg(sendto, string.format("Games %s Frags %s Deaths %s Kpd %s Accuracy %s Wins %s",
        yellow  (row.games),
        green   (row.frags),
        red     (row.deaths),
        yellow  (kpd),
        yellow  (acc .. "%"),
        green   (row.wins)))
end

stats_sub_command["total"] = function(cn, player)
    
    player = tonumber(player)
    if player and not server.valid_cn(player) then
        player = nil
    end
   
    player = player or cn

    return total_stats(cn, player)
end

return {initialize = initialize}
