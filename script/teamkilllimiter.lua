local teamkill_limit = server.teamkill_limit

function onTeamkill(actor, victim)
    if not server.player_var(actor,"shown_teamkill_warning") then
        if server.teamkill_showlimit == 1 then
            server.player_msg(actor,"This server will not tolerate more than " .. teamkill_limit .. " team kills per game.")
        else
        	server.player_msg(actor,"This server enforces a team kill limit, and so you need to play more carefully. You have been warned.")
        end
        server.player_var(actor,"shown_teamkill_warning",true)
    end
    if server.player_teamkills(actor) > teamkill_limit then
        server.kick(actor,1800,"server","teamkilling")
    end
end

server.event_handler("teamkill", onTeamkill)
