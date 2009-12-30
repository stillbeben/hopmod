function server.name_to_cn(name)

    if not name then
        return
    end
    
    name = string.lower(name)

    local name_matches = 0
    local first_matching_cn
    
    for i,cn in pairs(server.players()) do
    
        if string.lower(server.player_name(cn)) == name then
        
            first_matching_cn = first_matching_cn or cn
            name_matches = name_matches + 1
            
        end
        
    end
    
    if name_matches == 1 then
    
        return first_matching_cn
        
    elseif name_matches == 0 then
    
        local substring_matches = {}
        
        for i,cn in pairs(server.players()) do
            
            local candidate = server.player_name(cn)
            
            if string.find(string.lower(candidate), name) then
                table.insert(substring_matches, {name=candidate,cn=cn})
            end
        end
        
        return nil, substring_matches
        
    elseif name_matches > 1 then
        return nil, name_matches
    end
    
end

function server.disambiguate_name_list(cn, name)

	local message = ""

	name = string.lower(name)

	for p in server.gplayers() do
		local pname = p:name()

		if name == string.lower(pname) then
			message = message .. string.format("%i %s\n", p.cn, pname)
		end
	end

	server.player_msg(cn, message)

end

function server.similar_name_list(cn, names)

	local message = ""

	for i, player in pairs(names) do
		message = message .. string.format("%i %s\n", player.cn, player.name)
	end

	server.player_msg(cn, message)

end

function server.name_to_cn_list_matches(cn,name)

    local lcn, info = server.name_to_cn(name)

    if not lcn then

        if type(info) == "number" then  -- Multiple name matches

            server.player_msg(cn, red(string.format("There are %i players here matching that name:", info)))
	    server.disambiguate_name_list(cn,name)

        elseif #info == 0 then  -- no matches

            server.player_msg(cn, red("There are no players found matching that name."))

        else    -- Similar matches

	    server.player_msg(cn, red("There are no players found matching that name, but here are some similar names:"))
	    server.similar_name_list(cn, info)

        end

        return nil

    else

	return lcn
    end

end

function server.group_players(arg1,arg2,arg3)

    if not arg1 then
        return -1
    end

    local tag = nil
    local team = nil

    if arg1 == "all" then
	if not arg2 then
            return -1
	end

	tag = arg2

	if arg3 then
            team = arg3
	else
            team = tag
        end

	for j,cn in ipairs(server.spectators()) do
            if string.find(server.player_name(cn),tag) then
                server.unspec(cn)
            end
        end
    else
        tag = arg1

	if arg2 then
            team = arg2
        else
            team = tag
        end
    end

    for p in server.gplayers() do
        if string.find(p:name(),tag) then
            p:changeteam(team)
        end
    end
    
end


local alias_of_mode {
    "instagib team"     = "instagib team",
    "instateam"         = "instagib team",
    "iteam"             = "instagib team",
    "instagib"          = "instagib",
    "insta"             = "instagib",
    "insta ctf"         = "insta ctf",
    "instactf"          = "insta ctf",
    "ictf"              = "insta ctf",
    "ctf"               = "ctf",
    "insta protect"     = "insta protect",
    "instaprotect"      = "insta protect",
    "iprotect"          = "insta protect",
    "protect"           = "protect",
    "teamplay"          = "teamplay",
    "ffa"               = "ffa",
    "efficiency team"   = "efficiency team",
    "efficteam"         = "efficiency team",
    "eteam"             = "efficiency team",
    "efficiency"        = "efficiency",
    "effic"             = "efficiency",
    "tactics team"      = "tactics team",
    "tacteam"           = "tactics team",
    "tteam"             = "tactics team",
    "tactics"           = "tactics",
    "tac"               = "tactics",
    "regen capture"     = "regen capture",
    "regencapture"      = "regen capture",
    "regencap"          = "regen capture",
    "regen"             = "regen capture",
    "capture"           = "capture",
    "cap"               = "capture",
    "coop edit"         = "coop edit",
    "coopedit"          = "coop edit",
    "coop"              = "coop edit"
}

function server.parse_mode(mode)

    if mode then
        return alias_of_mode[mode]
    end

    return nil

end
