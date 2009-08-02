function server.playercmd_forcemap(forcemap_cn,forcemap_arg1,forcemap_arg2,forcemap_arg3)
    forcemap_map = "random"
    forcemap_mode = tostring(server.gamemode)
    forcemap_time = -1
    local function forcemap_do()
	if forcemap_arg1 then
	    if forcemap_arg1 == "instagib team" or forcemap_arg1 == "instateam" or forcemap_arg1 == "iteam" then
    		forcemap_mode = "instagib team"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "instagib" or forcemap_arg1 == "insta" then
    		forcemap_mode = "instagib"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "insta ctf" or forcemap_arg1 == "instactf" or forcemap_arg1 == "ictf" then
    		forcemap_mode = "insta ctf"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "ctf" then
    		forcemap_mode = "ctf"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "insta protect" or forcemap_arg1 == "instaprotect" or forcemap_arg1 == "iprotect" then
    		forcemap_mode = "insta protect"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "protect" then
    		forcemap_mode = "protect"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "teamplay" then
    		forcemap_mode = "teamplay"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "ffa" then
    		forcemap_mode = "ffa"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "efficiency team" or forcemap_arg1 == "efficteam" or forcemap_arg1 == "eteam" then
    		forcemap_mode = "efficiency team"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "efficiency" or forcemap_arg1 == "effic" then
    		forcemap_mode = "efficiency"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "tactics team" or forcemap_arg1 == "tacteam" or forcemap_arg1 == "tteam" then
    		forcemap_mode = "tactics team"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "tactics" or forcemap_arg1 == "tac" then
    		forcemap_mode = "tactics"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "regencapture" or forcemap_arg1 == "regencapture" or forcemap_arg1 == "regen" then
    		forcemap_mode = "regen capture"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "capture" then
    		forcemap_mode = "capture"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 == "coop edit" or forcemap_arg1 == "coop" then
    		forcemap_mode = "coop edit"
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			forcemap_map = forcemap_arg2
    			if forcemap_arg3 then
    			    if forcemap_arg3 > "0" and forcemap_arg3 <= "13669" then
    				forcemap_time = tonumber(forcemap_arg3)
    			    else
    				server.player_msg(forcemap_cn,red("time is not valid"))
    				return 0
    			    end
    			end
    		    end
    		end
    	    elseif forcemap_arg1 > "0" and forcemap_arg1 <= "13669" then
    		forcemap_time = tonumber(forcemap_arg1)
    	    else
    		forcemap_map = forcemap_arg1
    		if forcemap_arg2 then
    		    if forcemap_arg2 > "0" and forcemap_arg2 <= "13669" then
    			forcemap_time = tonumber(forcemap_arg2)
    		    else
    			server.player_msg(forcemap_cn,red("time is not valid"))
    			return 0
    		    end
    		end
    	    end
    	end
    	if forcemap_map == "random" then
    	    forcemap_map = server.random_map(forcemap_mode,0)
    	end
    end
    
    local tmp = forcemap_do()
    if tmp == 0 then
        return
    end
    if server.player_priv_code(forcemap_cn) > 1 then
	server.changemap(forcemap_map,forcemap_mode,forcemap_time)
    elseif server.player_priv_code(forcemap_cn) > 0 then
	server.changemap(forcemap_map,forcemap_mode,-1)
    else
	server.player_msg(forcemap_cn,red("permission denied"))
    end
end

function server.playercmd_fmap(cn,fmap_arg1,fmap_arg2,fmap_arg3)
    server.playercmd_forcemap(cn,fmap_arg1,fmap_arg2,fmap_arg3)
end
