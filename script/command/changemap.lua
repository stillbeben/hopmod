-- #changemap [<mode>] [<map>] [<time>]

function server.playercmd_changemap(changemap_cn,arg1,arg2,arg3)
    changemap_map = "random"
    changemap_mode = tostring(server.gamemode)
    changemap_time = -1
    if arg1 then
	if arg1 == "instagib team" or arg1 == "instateam" or arg1 == "iteam" or arg1 == "instagib" or arg1 == "insta" or arg1 == "insta ctf" or arg1 == "instactf" or arg1 == "ictf" or arg1 == "ctf" or arg1 == "insta protect" or arg1 == "instaprotect" or arg1 == "iprotect" or arg1 == "protect" or arg1 == "teamplay" or arg1 == "ffa" or arg1 == "efficiency team" or arg1 == "efficteam" or arg1 == "eteam" or arg1 == "efficiency" or arg1 == "effic" or arg1 == "tactics team" or arg1 == "tacteam" or arg1 == "tteam" or arg1 == "tactics" or arg1 == "tac" or arg1 == "regencapture" or arg1 == "regencapture" or arg1 == "regen" or arg1 == "capture" then
    	    if arg2 then
    		if not ( arg2 > "0" and arg2 <= "13669" ) then
    		    changemap_map = arg2
    		end
    	    end
    	elseif not ( arg1 > "0" and arg1 <= "13669" ) then
    	    changemap_map = arg1
    	end
    	if changemap_map ~= "random" then
    	    if not server.is_known_map(changemap_map) then
    	        server.player_msg(changemap_cn,red("map is not known"))
    	        return
    	    end
    	end
	server.playercmd_forcemap(changemap_cn,arg1,arg2,arg3)
    end
    server.playercmd_forcemap(changemap_cn,arg1,arg2,arg3)
end

function server.playercmd_cmap(cmap_cn,arg1,arg2,arg3)
    server.playercmd_changemap(cmap_cn,arg1,arg2,arg3)
end
