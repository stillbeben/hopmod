local function random_init()
    local rand_non_null_value = (tonumber(server.serverport) * tonumber(server.gamelimit)) + tonumber(server.serverport)
    math.randomseed(math.abs((((rand_non_null_value * os.time()) / os.clock()) * (os.time()/os.clock())) % rand_non_null_value))
end


random_init()


function server.random_map(random_map_mode,random_map_small)
    if not random_map_small then
	random_map_small = 1
    end
    
    local random_map_list = {}
    local parse = server.parse_list
    if random_map_small == 1 and ( random_map_mode == "ffa" or random_map_mode == "teamplay" or random_map_mode == "efficiency" or random_map_mode == "efficiency team" or random_map_mode == "tactics" or random_map_mode == "tactics team" or random_map_mode == "instagib" or random_map_mode == "instagib team" ) then
	random_map_list = table_unique(parse(server.small_maps))
    else
	random_map_list = table_unique(parse(server[random_map_mode .. "_maps"]))
    end
    if not random_map_list then
	return nil
    end
    
    return random_map_list[math.random(#random_map_list)]
end
