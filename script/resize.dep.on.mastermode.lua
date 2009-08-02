local function resizeMastermode(mmode)
    if not mmode then
	mmode = server.mastermode
	if mmode == 0 then
	    mmode = "open"
	elseif mmode == 1 then
	    mmode = "veto"
	elseif mmode == 2 then
	    mmode = "locked"
	elseif mmode == 3 then
	    mmode = "private"
	else
	    return
	end
    end
    if mmode == server.resize_mastermode then
        server.maxplayers = server.resize_totalmaxplayers
    else
        server.maxplayers = resize_mastermode_normalmaxplayers
    end
end

server.event_handler("setmastermode",function(old,new)
    resizeMastermode(new)
end)

server.event_handler("disconnect",function(cn)
    if tonumber(server.playercount) == 0 then
	resizeMastermode()
    end
end)

resize_mastermode_normalmaxplayers = tonumber(server.maxplayers)
resizeMastermode()
