
local gamecount = 0
local maps = {}
local def_maps = {}
local modes = nil
local sizedmaps = {}
local bestmapsize = 0
local smallgamesize = 0

function server.reload_maprotation()

    local parse = server.parse_list
    
    maps["ffa"] = table_unique(parse(server["ffa_maps"]))
    maps["templay"] = table_unique(parse(server["teamplay_maps"]))
    maps["efficiency"] = table_unique(parse(server["efficiency_maps"]))
    maps["efficiency team"] = table_unique(parse(server["efficiency team_maps"]))
    maps["tactics"] = table_unique(parse(server["tactics_maps"]))
    maps["tactics team"] = table_unique(parse(server["tactics team_maps"]))
    maps["instagib"] = table_unique(parse(server["instagib_maps"]))
    maps["instagib team"] = table_unique(parse(server["instagib team_maps"]))
    maps["capture"] = table_unique(parse(server["capture_maps"]))
    maps["regen capture"] = table_unique(parse(server["regen capture_maps"]))
    maps["ctf"] = table_unique(parse(server["ctf_maps"]))
    maps["insta ctf"] = table_unique(parse(server["insta ctf_maps"]))
    maps["protect"] = table_unique(parse(server["protect_maps"]))
    maps["insta protect"] = table_unique(parse(server["insta protect_maps"]))
    
    sizedmaps.big_maps = table_unique(parse(server.big_maps))
    sizedmaps.small_maps = table_unique(parse(server.small_maps))
    bestmapsize = server.use_best_map_size
    smallgamesize = server.small_gamesize
    
    def_maps["ffa"] = table_unique(parse(server["def_ffa_maps"]))
    def_maps["templay"] = table_unique(parse(server["def_teamplay_maps"]))
    def_maps["efficiency"] = table_unique(parse(server["def_efficiency_maps"]))
    def_maps["efficiency team"] = table_unique(parse(server["def_efficiency team_maps"]))
    def_maps["tactics"] = table_unique(parse(server["def_tactics_maps"]))
    def_maps["tactics team"] = table_unique(parse(server["def_tactics team_maps"]))
    def_maps["instagib"] = table_unique(parse(server["def_instagib_maps"]))
    def_maps["instagib team"] = table_unique(parse(server["def_instagib team_maps"]))
    def_maps["capture"] = table_unique(parse(server["def_capture_maps"]))
    def_maps["regen capture"] = table_unique(parse(server["def_regen capture_maps"]))
    def_maps["ctf"] = table_unique(parse(server["def_ctf_maps"]))
    def_maps["insta ctf"] = table_unique(parse(server["def_insta ctf_maps"]))
    def_maps["protect"] = table_unique(parse(server["def_protect_maps"]))
    def_maps["insta protect"] = table_unique(parse(server["def_insta protect_maps"]))
    
    modes = table_unique(parse(server["game_modes"]))
end

local function nextmap(gmode,i)
    maplist = maps[gmode]
    
    if bestmapsize and not server.gamemodeinfo.teams then
        if tonumber(server.playercount) <= smallgamesize then
            maplist = sizedmaps.small_maps
        else
            maplist = sizedmaps.big_maps
        end
    end
    
    if not maplist then
	return nil
    end
    local nm = maplist[(i % #maplist)+1]
    if nm == tostring(server.map) then
	return nextmap(gmode,i+1)
    end
    return nm
end

function server.is_known_map(mapname, gamemode)
    
    if not gamemode then
        for k,list in pairs(maps) do
            if server.is_known_map(mapname, k) then return true end
        end
        
        return false
    end
    
    for i,mapnameX in ipairs(maps[gamemode]) do
        if mapname == mapnameX then return true end
    end
    
    return false
end

local function get_mode()
    if server.use_server_random_moderotation == 1 then
	local gmode = server.random_mode()
	if gmode then
	    while tostring(server.gamemode) == gmode do
		gmode = server.random_mode()
	    end
	    return gmode
	end
    else
	return tostring(server.gamemode)
    end
    
    return nil
end

server.event_handler("setnextgame",function()
    
    local use_first_map = server.use_first_map
    
    if server.use_server_maprotation == 1 then
	if server.use_server_random_maprotation == 1 then
	    server.next_mode = get_mode()
	    local nm = server.random_map(tostring(server.next_mode),bestmapsize)
	    if nm then
		while nm == tostring(server.map) do
		    nm = server.random_map(tostring(server.next_mode),bestmapsize)
		end
		server.next_map = nm
	    end
	else
	    server.next_mode = get_mode()
	    local nm = nextmap(tostring(server.next_mode), gamecount)
	    if nm then
		server.next_map = nm
	    end
	end
    end
    
    gamecount = gamecount + 1
end)

local function def_nextmap(gmode,cmap)
    local mlist = def_maps[gmode]
    
    if not mlist then
	return nil
    end
    
    local index = -1
    for i,name in ipairs(mlist) do
	if name == cmap then
	    index = i
	    break
	end
    end
    
    if index == -1 then
	return nil
    end
    
    return mlist[(index % #mlist) + 1]
end

-- #nextmap
if server.enable_nextmap_command == 1 then
    function server.playercmd_nextmap(cn)
	if server.use_server_maprotation == 1 then
	    if server.use_server_random_moderotation == 1 then
		server.player_msg(cn, "Note: The next mode will be chosen randomly at the end of this game, therefore the next map, too.")
	    elseif server.use_server_random_maprotation == 1 then
		server.player_msg(cn, "The next map will be chosen randomly at the end of this game.")
	    else
		local nm = nextmap(tostring(server.gamemode), gamecount)
		if nm then
		    server.player_msg(cn, "The next map is " .. green(nm) .. ".")
		    if bestmapsize and not server.gamemodeinfo.teams then
			server.player_msg(cn, "Note: The next map will be determined on the number of players still connected at the end of this game.")
		    end
		end
	    end
	else
	    local nm = def_nextmap(tostring(server.gamemode),tostring(server.map))
	    if nm then
		server.player_msg(cn, "The next map is " .. green(nm) .. ".")
	    end
	end
	return
    end
end
