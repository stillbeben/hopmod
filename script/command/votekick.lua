-- #votekick <cn>|"<name>"
-- [[ based on a player command written by Thomas ]] --

local varvk = {}

server.event_handler("connect",function(cn)
    varvk[cn] = {}
end)

server.event_handler("disconnect",function(cn,reason)
    varvk[cn] = nil
end)

if server.votekick_ad_timer ~= 0 then
    server.interval(server.votekick_ad_timer,function()
	if (tonumber(server.playercount) > 2) and (tonumber(server.mastermode) == 0) then
	    server.msg("(" .. green("Info") .. ")  If you " .. orange("detect a cheater") .. ", type: " .. yellow("#votekick \"name\"") .. " or " .. yellow("#votekick cn"))
	end
    end)
end

server.playercmd_votekick = function(cn,cn_kick)
    if tonumber(server.playercount) < 3 then
        server.player_msg(cn,red("votekick works, when there are more than 3 players"))
        return
    end
    if not cn_kick then
	server.player_msg(cn,red("#votekick (<cn>|\"<name>\")"))
        return
    end
    if not server.valid_cn(cn_kick) then
        cn_kick = server.find_cn(cn,cn_kick)
	if not cn_kick then
	    return
	end
    end
    cn = tonumber(cn)
    cn_kick = tonumber(cn_kick)
    if cn_kick == cn then
        server.player_msg(cn,red("don't kick yourself"))
        return
    end
    if varvk[cn_kick][cn] then
        server.player_msg(cn,orange("you have already voted to kick this player"))
	return
    end
    varvk[cn_kick][cn] = true
    if not varvk[cn_kick].votes then
	varvk[cn_kick].votes = 0
    end
    varvk[cn_kick].votes = varvk[cn_kick].votes + 1
    server.msg(green(server.player_name(cn)) .. " voted to kick " .. red(server.player_name(cn_kick)))
    local required_votes = round((server.playercount / 2), 0)
    server.msg("Votes: " .. varvk[cn_kick].votes .. " of " .. required_votes)
    if varvk[cn_kick].votes >= required_votes then
        server.kick(cn_kick,3600,"server","votekick")
        server.msg("(" .. green("Info") .. ")  player was kicked by votekick")
        varvk[cn_kick] = nil
    end
end
