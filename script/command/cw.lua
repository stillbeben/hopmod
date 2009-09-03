-- #cw <team1> <team2> <mode> <map> [<time>]

local function cw_teamsize(cw_teamsize_teamname)
    cw_teamsize_i = 0
    for a,cn in ipairs(server.team_players(cw_teamsize_teamname)) do
        cw_teamsize_i = cw_teamsize_i + 1
    end
    return cw_teamsize_i
end

local cmd_cw = {}

function cmd_cw.playercmd(cw_cn,cw_team1,cw_team2,cw_mode,cw_map,cw_maptime)
    if not cw_team1 then
	server.player_msg(cw_cn,red("a teamname is missing"))
	return
    elseif not cw_team2 then
        server.player_msg(cw_cn,red("a teamname is missing"))
        return
    elseif not cw_mode then
        server.player_msg(cw_cn,red("mode is missing"))
        return
    elseif not cw_map then
        server.player_msg(cw_cn,red("map is missing"))
        return
    end
    cw_lteam1 = cw_team1
    cw_lteam2 = cw_team2
    cw_lmap = cw_map
    cw_lmaptime = cw_maptime
    if cw_mode == "insta" or cw_mode == "instateam" then
        cw_lmode = "instagib team"
    elseif cw_mode == "ffa" or cw_mode == "teamplay" then
        cw_lmode = "teamplay"
    elseif cw_mode == "effic" or cw_mode == "efficteam" then
        cw_lmode = "efficiency team"
    elseif cw_mode == "tac" or cw_mode == "tacteam" then
        cw_lmode = "tactics team"
    elseif cw_mode == "instactf" then
        cw_lmode = "insta ctf"
    elseif cw_mode == "instaprotect" then
        cw_lmode = "insta protect"
    elseif cw_mode == "ctf" then
	cw_lmode = "ctf"
    elseif cw_mode == "capture" then
        cw_lmode = "capture"
    elseif cw_mode == "regencapture" then
        cw_lmode = "regen capture"
    elseif cw_mode == "protect" then
        cw_lmode = "protect"
    else
        server.player_msg(cw_cn,red("gamemode isn't known"))
        return
    end
    
    server.mastermode = 2
    
    if cw_lmode == "insta ctf" or cw_lmode == "ctf" or cw_lmode == "insta protect" or cw_lmode == "protect" then
        server.playercmd_group(cw_cn,cw_lteam1,"evil")
        server.playercmd_group(cw_cn,cw_lteam2,"good")
    else
        server.playercmd_group(cw_cn,cw_lteam1)
        server.playercmd_group(cw_cn,cw_lteam2)
    end
    
    server.log(string.format("MATCH: %s versus %s on %s (%s)",cw_lteam1,cw_lteam2,cw_lmap,cw_lmode))
    server.msg(string.format("in 10 seconds: %s versus %s on %s (%s)",green(cw_lteam1),green(cw_lteam2),green(cw_lmap),green(cw_lmode)))
    
    server.sleep(5000,function()
        server.msg("Game will start in 5 seconds...")
        server.sleep(1000,function()
            server.msg("Game will start in 4 seconds...")
            server.sleep(1000,function()
                server.msg("Game will start in 3 seconds...")
                server.sleep(1000,function()
                    server.msg("Game will start in 2 seconds...")
                    server.sleep(1000,function()
                        server.msg("Game will start in 1 second...")
                        server.msg(string.format("map will change to: %s (%s)",green(cw_lmap),green(cw_lmode)))
                        server.sleep(1500,function()
                            cw_player_counter = tonumber(server.playercount) - tonumber(server.speccount)
                            server.changemap(cw_lmap,cw_lmode,-1)
                      	    server.pausegame(true)
                      	    cw_active = server.event_handler("active",function(cn)
                      		cw_player_counter = cw_player_counter - 1
                      		if cw_player_counter == 0 then
                      		    server.pausegame(false)
                      		    server.msg(green("Good luck and have fun!"))
                      		    server.cancel_handler(cw_active)
                      		end
    	  		    end)
    	                    
    			    if cw_lmode == "insta ctf" or cw_lmode == "ctf" or cw_lmode == "insta protect" or cw_lmode == "protect" then
    			        server.playercmd_group(cw_cn,cw_lteam1,"e")
    			        server.playercmd_group(cw_cn,cw_lteam2,"g")
    			    else
    			        server.playercmd_group(cw_cn,cw_lteam1,cw_lteam1)
    			        server.playercmd_group(cw_cn,cw_lteam2,cw_lteam2)
    			    end
    			    if cw_lmaptime then
    	                	server.changetime(cw_lmaptime * 60000)
    	                    else
    	                        server.changetime(600000)
    	                    end
    	                    
    			    server.msg("[Start demo recording]")
    			    server.recorddemo("log/demo/" .. cw_team1 .. ".vs." .. cw_team2 .. "." .. cw_mode .. "." .. cw_map .. ".dmo")
			    server.msg(orange("Start the fight now!"))
			    cw_reteam_disabled = 0
			    cw_reteam = server.event_handler("reteam",function(cn,old,new)
				if cw_reteam_disabled == 0 then
				    cw_reteam_cn = cn
				    cw_reteam_old = old
				    cw_reteam_active = 1
				    server.sleep(120,function()
					if cw_reteam_active == 1 then
					    server.changeteam(cw_reteam_cn,cw_reteam_old)
					    cw_reteam_active = 0
					end
				    end)
				end
			    end)
			    cw_disconnect = server.event_handler("disconnect",function(cn)
				if cw_lmode == "insta ctf" or cw_lmode == "ctf" or cw_lmode == "insta protect" or cw_lmode == "protect" then
				    cw_disconnect_lteam1 = "evil"
				    cw_disconnect_lteam2 = "good"
				else
				    cw_disconnect_lteam1 = cw_lteam1
				    cw_disconnect_lteam2 = cw_lteam2
				end
				if not (server.player_status_code(cn) == 5) then
--			    	   if not ( cw_teamsize(cw_disconnect_lteam1) == cw_teamsize(cw_disconnect_lteam2) ) then
				    server.pausegame(true)
				    server.msg(" ")
				    server.msg(orange("One player has disconnected - game is paused..."))
				    server.msg(" ")
				    cw_reteam_disabled = 1
				    server.sleep(240000,function()
					cw_reteam_disabled = 0
				    end)
				end
			    end)
			    cw_la = 1
                    	    cw_lb = 1
                    	    cw_lc = 1
                    	    cw_ld = 1
    	            	    cw_intermission = server.event_handler("intermission",function()
    	                	if cw_la == 1 then
    	                    	    server.msg(orange("Good Game!"))
    	                    	    cw_la = 0
    	                	end
    	                	if cw_lmode == "insta ctf" or cw_lmode == "ctf" or cw_lmode == "insta protect" or cw_lmode == "protect" then
    	                    	    cw_intermission_lteam1 = "evil"
    	                    	    cw_intermission_lteam2 = "good"
    	                	else
    	                	    cw_intermission_lteam1 = cw_lteam1
    	                	    cw_intermission_lteam2 = cw_lteam2
    	                	end
    	                	if server.team_score(cw_intermission_lteam1) == server.team_score(cw_intermission_lteam2) then
    	                    	    if cw_lb == 1 then
    	                    		server.log(string.format("MATCH: draw: %s versus %s [%s]",cw_lteam1,cw_lteam2,server.team_score(cw_intermission_lteam1)))
    	                    		server.msg(string.format("Game - %s versus %s - ended with: draw: %s - %s",green(cw_lteam1),green(cw_lteam2),green(server.team_score(cw_lteam1)),green(server.team_score(cw_lteam2))))
    	                    		cw_lb = 0
    	                    	    end
    	                	elseif server.team_score(cw_intermission_lteam1) > server.team_score(cw_intermission_lteam2) then
    	                    	    cw_winner = cw_intermission_lteam1
    	                    	    cw_winner_team = cw_lteam1
    	                    	    cw_loser = cw_intermission_lteam2
    	                    	    cw_loser_team = cw_lteam2
    	                	else
    	                	    cw_winner = cw_intermission_lteam2
    	                	    cw_winner_team = cw_lteam2
    	                    	    cw_loser = cw_intermission_lteam1
    	                    	    cw_loser_team = cw_lteam1
    	                	end
    	                	if not ( server.team_score(cw_intermission_lteam1) == server.team_score(cw_intermission_lteam2) ) then
    	                    	    if cw_lc == 1 then
    	                    		server.log(string.format("MATCH: %s wins with %s - %s has %s",cw_winner_team,server.team_score(cw_winner_team),cw_loser_team,server.team_score(cw_loser_team)))
    	                    		server.msg(string.format("Game ended with: %s wins with %s - %s has %s",green(cw_winner_team),green(server.team_score(cw_winner)),green(cw_loser_team),green(server.team_score(cw_loser))))
    	                    		cw_lc = 0
    	                    	    end
    	                	else
    	                    	    cw_lc = 0
    	                	end
    	                	if cw_ld == 1 then
    	                	    server.msg(string.format("[Stop demo recording]   (get demo with %s)",yellow("/getdemo")))
                        	    cw_ld = 0
    	                	end
    	                	server.stopdemo()
    	                	cw_reteam_disabled = 1
                        	server.cancel_handler(cw_reteam)
    	                	server.cancel_handler(cw_disconnect)
    	                	server.sleep(8000,function()
    	                    	    server.playercmd_group(cw_cn,cw_lteam1,"evil")
    	                    	    server.playercmd_group(cw_cn,cw_lteam2,"good")
    	                	end)
--    	                    	server.mastermode = 1
				server.cancel_handler(cw_active)
    	                	server.cancel_handler(cw_intermission)
			    end)
    	        	end)
    	    	    end)
    		end)
    	    end)
	end)
    end)
end

if server.cw_command_master == 1 then
    function server.playercmd_cw(cw_cn,cw_team1,cw_team2,cw_mode,cw_map,cw_maptime)
	return mastercmd(function()
	    cmd_cw.playercmd(cw_cn,cw_team1,cw_team2,cw_mode,cw_map,cw_maptime)
	end,cw_cn)
    end
else
    function server.playercmd_cw(cw_cn,cw_team1,cw_team2,cw_mode,cw_map,cw_maptime)
	return admincmd(function()
	    cmd_cw.playercmd(cw_cn,cw_team1,cw_team2,cw_mode,cw_map,cw_maptime)
	end,cw_cn)
    end
end
