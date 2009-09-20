local teamkill_limit = server.teamkill_limit

function onTeamkill(actor, victim)

    if not server.player_var(actor,"shown_teamkill_warning") then
    
        if server.teamkill_showlimit == 1 then
            server.player_msg(actor,"This " .. orange("server will not tolerate") .. " more than " .. teamkill_limit .. orange(" team kills") .. " per game.")
    	else
    	    server.player_msg(actor,"This " .. orange("server enforces a team kill limit") .. ", and so you need to " .. orange("play more carefully") .. ". You have been warned.")
    	end
        
    	server.player_var(actor, "shown_teamkill_warning", true)
    end
    
    if server.teamkill_show_public == 1 then
        server.msg("(" .. green("Info") .. ")  " .. green(server.player_name(actor)) .. "(" .. magenta(actor) .. ") " .. orange("killed a teammate."))
    end
    
    if server.player_teamkills(actor) > teamkill_limit then
        server.kick(actor,server.teamkill_bantime,"server","teamkilling")
    end
end

local teamkill_event = server.event_handler("teamkill", onTeamkill)

if server.teamkill_showlimit == 1 then
    player_command_script("teamkills", "./script/command/teamkills.lua")
end


return {unload = function() server.cancel_handler(teamkill_event) end}
