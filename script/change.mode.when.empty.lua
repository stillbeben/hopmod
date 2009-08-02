server.event_handler("disconnect",function(cn)
    if tonumber(server.playercount) == 0 then
	if server.gamemode == "capture" then
	    server.changemap("memento","instagib",-1)
	elseif server.gamemode == "instagib" then
	    server.changemap("memento","efficiency team",-1)
	elseif server.gamemode == "efficiency team" then
	    server.changemap("dust2","ctf",-1)
	elseif server.gamemode == "ctf" then
	    server.changemap("memento","tactics",-1)
	elseif server.gamemode == "tactics" then
	    server.changemap("memento","teamplay",-1)
	elseif server.gamemode == "teamplay" then
	    server.changemap("dust2","regen capture",-1)
	elseif server.gamemode == "regen capture" then
	    server.changemap("dust2","insta protect",-1)
	elseif server.gamemode == "insta protect" then
	    server.changemap("memento","ffa",-1)
	elseif server.gamemode == "ffa" then
	    server.changemap("memento","tactics team",-1)
	elseif server.gamemode == "tactics team" then
	    server.changemap("dust2","insta ctf",-1)
	elseif server.gamemode == "insta ctf" then
	    server.changemap("memento","efficiency",-1)
	elseif server.gamemode == "efficiency" then
	    server.changemap("memento","instagib team",-1)
	elseif server.gamemode == "instagib team" then
	    server.changemap("dust2","protect",-1)
	elseif server.gamemode == "protect" then
	    server.changemap("dust2","capture",-1)
	else
	    server.changemap(server.first_map,server.first_gamemode,-1)
	end
    end
end)

	
