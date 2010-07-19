-- #1on1 <cn1> <cn2> <mode> <map> [<maptime>]

local cmd_oneonone = {}

return function(oneonone_cn,oneonone_cn1,oneonone_cn2,oneonone_mode,oneonone_map,oneonone_maptime)
    if not oneonone_cn1 then
        server.player_msg(oneonone_cn,red("a cn is missing"))
        return
    elseif not oneonone_cn2 then
        server.player_msg(oneonone_cn,red("a cn is missing"))
        return
    elseif not oneonone_mode then
        server.player_msg(oneonone_cn,red("mode is missing"))
        return
    elseif not oneonone_map then
        server.player_msg(oneonone_cn,red("map is missing"))
        return
    end
    if not ( server.valid_cn(oneonone_cn1) and server.valid_cn(oneonone_cn2) ) then
        server.player_msg(oneonone_cn,red("a cn is not a number"))
        return
    end
    oneonone_lcn1 = oneonone_cn1
    oneonone_lcn2 = oneonone_cn2
    oneonone_lmap = oneonone_map
    oneonone_lmaptime = oneonone_maptime
    if oneonone_mode == "insta" then
        oneonone_lmode = "instagib"
    elseif oneonone_mode == "instateam" then
        oneonone_lmode = "instagib team"
    elseif oneonone_mode == "ffa" then
        oneonone_lmode = "ffa"
    elseif oneonone_mode == "teamplay" then
        oneonone_lmode = "teamplay"
    elseif oneonone_mode == "effic" then
        oneonone_lmode = "efficiency"
    elseif oneonone_mode == "efficteam" then
        oneonone_lmode = "efficiency team"
    elseif oneonone_mode == "tac" then
        oneonone_lmode = "tactics"
    elseif oneonone_mode == "tacteam" then
        oneonone_lmode = "tactics team"
    elseif oneonone_mode == "instactf" then
        oneonone_lmode = "insta ctf"
    elseif oneonone_mode == "instaprotect" then
        oneonone_lmode = "insta protect"
    elseif oneonone_mode == "ctf" then
        oneonone_lmode = "ctf"
    elseif oneonone_mode == "capture" then
        oneonone_lmode = "capture"
    elseif oneonone_mode == "regencapture" then
        oneonone_lmode = "regen capture"
    elseif oneonone_mode == "protect" then
        oneonone_lmode = "protect"
    else
        server.player_msg(oneonone_cn,red("gamemode isn't known"))
        return
    end
    
    oneonone_player1con = server.player_sessionid(oneonone_lcn1)
    oneonone_player2con = server.player_sessionid(oneonone_lcn2)
    
    if not ( oneonone_lmode == "instagib" or oneonone_lmode == "efficiency" or oneonone_lmode == "tactics" or oneonone_lmode == "ffa" ) then
        server.changeteam(oneonone_lcn1,"evil")
        server.changeteam(oneonone_lcn2,"good")
    end
    
    server.mastermode = 2
    server.specall()
    server.unspec(oneonone_lcn1)
    server.unspec(oneonone_lcn2)
    
    server.log(string.format("MATCH: %s versus %s on %s (%s)",server.player_name(oneonone_lcn2),server.player_name(oneonone_lcn1),oneonone_lmap,oneonone_lmode))
    server.msg(string.format("in 10 seconds: %s versus %s on %s (%s)",green(server.player_name(oneonone_lcn1)),green(server.player_name(oneonone_lcn2)),green(oneonone_lmap),green(oneonone_lmode)))
    
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
                        server.msg(string.format("map will change to: %s (%s)",green(oneonone_lmap),green(oneonone_lmode)))
                        server.sleep(1500,function()
                            server.changemap(oneonone_lmap,oneonone_lmode,-1)
                            if oneonone_lmaptime then
                    	        server.changetime(oneonone_lmaptime * 60000)
			    else
                        	server.changetime(600000)
    	                    end
			    if not ( oneonone_lmode == "instagib" or oneonone_lmode == "efficiency" or oneonone_lmode == "tactics" or oneonone_lmode == "ffa" ) then
				server.changeteam(oneonone_lcn1,"evil")
				server.changeteam(oneonone_lcn2,"good")
			    end
			    server.msg(orange("Start the fight now!"))
			    server.msg(green("Good luck and have fun!"))
			    oneonone_la = 1
			    oneonone_lb = 1
			    oneonone_lc = 1
			    oneonone_ld = 1
    	            	    oneonone_intermission = server.event_handler("intermission",function()
    	                        if oneonone_player1con == server.player_sessionid(oneonone_lcn1) and oneonone_player2con == server.player_sessionid(oneonone_lcn2) then
    	                	    if oneonone_la == 1 then
    	                                server.msg(orange("Good Game!"))
    	                                oneonone_la = 0
    	                            end
    	                            if oneonone_lmode == "instagib" or oneonone_lmode == "efficiency" or oneonone_lmode == "tactics" or oneonone_lmode == "ffa" then
    	                                oneonone_p1_frags = server.player_frags(oneonone_lcn1)
    	                                oneonone_p2_frags = server.player_frags(oneonone_lcn2)
    	                            else
    	                                oneonone_p1_frags = server.team_score("evil")
    	                                oneonone_p2_frags = server.team_score("good")
    	                            end
    	                            if oneonone_p1_frags == oneonone_p2_frags then
    	                                if oneonone_lb == 1 then
    	                            	    server.log(string.format("MATCH: draw: %s versus %s [%s]",server.player_name(oneonone_lcn1),server.player_name(oneonone_lcn2),oneonone_p1_frags))
    	                            	    server.msg(string.format("Game - %s versus %s - ended with: draw: %s - %s",green(server.player_name(oneonone_lcn1)),green(server.player_name(oneonone_lcn2)),green(oneonone_p1_frags),green(oneonone_p2_frags)))
    	                            	    oneonone_lb = 0
    	                        	end
    	                    	    elseif oneonone_p1_frags > oneonone_p2_frags then
    	                        	oneonone_winner = oneonone_lcn1
    	                            	oneonone_winner_score = oneonone_p1_frags
    	                                oneonone_loser = oneonone_lcn2
    	                                oneonone_loser_score = oneonone_p2_frags
    	                    	    else
    	                    	        oneonone_winner = oneonone_lcn2
    	                                oneonone_winner_score = oneonone_p2_frags
    	                                oneonone_loser = oneonone_lcn1
    	                                oneonone_loser_score = oneonone_p1_frags
    	                            end
    	                            if oneonone_lc == 1 then
    	                                server.log(string.format("MATCH: %s wins with %s - %s has %s",server.player_name(oneonone_winner),oneonone_winner_score,server.player_name(oneonone_loser),oneonone_loser_score))
    	                                server.msg(string.format("Game ended with: %s wins with %s - %s has %s",green(server.player_name(oneonone_winner)),green(oneonone_winner_score),green(server.player_name(oneonone_loser)),green(oneonone_loser_score)))
    	                                oneonone_lc = 0
    	                            end
    	                        else
    	                    	    if oneonone_ld == 1 then
    	                    	        server.msg(orange("Can't display the score. Maybe one of the players has been disconnected during the game!"))
    	                                server.msg(orange("But you can take a look at the scoreboard!"))
    	                                oneonone_ld = 0
    	                            end
    	                        end
    	                    	server.cancel_handler(oneonone_intermission)
			    end)
    	                end)
    	            end)
    	        end)
    	    end)
    	end)
    end)
end

