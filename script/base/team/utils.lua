function server.teams_count()

    local size = 0
    
    for _, team in pairs(server.teams())
    do
	size = size + 1
    end
    
    return size
end

function server.team_size(team, with_specs, with_bots)

    if not team
    then
	return
    end
    
    if with_specs and with_bots
    then
	return server.teamsize(team)
    end
    
    local size = 0
    
    for p in server.gteamplayers(team, with_specs, with_bots)
    do
        size = size + 1
    end
    
    return size
end

local function compare_two_teams_by_size(lower_than, team1, team2, with_specs, with_bots)

    local size1 = server.team_size(team1, with_specs, with_bots)
    local size2 = server.team_size(team2, with_specs, with_bots)
    
    if size1 == size2
    then
        return
    end
    
    if size1 > size2
    then
	if lower_than
	then
	    return team2
	end    
	
        return team1
    end
    
    if lower_than
    then
	return team1
    end
    
    return team2
end

function server.bigger_team(team1, team2, with_specs, with_bots)

    if not team1 or not team2
    then
	team1 = "good"
	team2 = "evil"
    end
    
    return compare_two_teams_by_size(nil, team1, team2, with_specs, with_bots)
end

function server.smaller_team(team1, team2, with_specs, with_bots)

    if not team1 or not team2
    then
	team1 = "good"
	team2 = "evil"
    end
    
    return compare_two_teams_by_size(true, team1, team2, with_specs, with_bots)
end

-- returns 'nil, list', when teamsizes are identical
local function compare_teams_by_size(lower_than, with_specs, with_bots)

    local tcount = server.teams_count()
    
    if tcount < 2
    then
	for _, team in pairs(server.teams())
	do
	    return team
	end
	
    elseif tcount == 2
    then
	local teams = {}
	
	for _, team in pairs(server.teams())
	do
	    table.insert(teams, team)
	end
	
	server.compare_two_teams_by_size(lower_than, teams[0], teams[1], with_specs, with_bots)
	
    else
	local sizes	= {}
	local sizes_map	= {}
	local doubler	= {}
	
	for _, team in pairs(server.teams())
	do
	    local size = server.team_size(team, with_specs, with_bots)
	    
	    if sizes_map[size]
	    then
		if not doubler[size]
		then
	    	    doubler[size] = {}
	    	    table.insert(doubler[size], sizes_map[size])
	    	    sizes_map[size] = "012345"
		end
		
		table.insert(doubler[size], team)
	    else
		sizes_map[size] = team
		table.insert(sizes, size)
	    end
	end
	
	if lower_than
	then
	    table.sort(sizes)
	else
	    table.sort(sizes, function(a, b)
	
		    if a > b
		    then
			return true
		    end
		    
		    return false
		end)
	end
	
	if sizes_map[sizes[0]] == "012345"
	then
	    return nil, doubler[sizes[0]]
	end
	
	return sizes_map[sizes[0]]
    end
end

function server.biggest_team(with_specs, with_bots)

    return compare_teams_by_size(nil, with_specs, with_bots)
end

function server.smallest_team(with_specs, with_bots)

    return compare_teams_by_size(true, with_specs, with_bots)
end

function server.team_size_difference(team1, team2, with_specs, with_bots)

    if not team1 or not team2
    then
	team1 = "good"
	team2 = "evil"
    end
    
    return (math.abs(server.team_size(team1, with_specs, with_bots) - server.team_size(team2, with_specs, with_bots)))
end

local function compare_two_teams_by_diff(diff, team1, team2, with_specs, with_bots)

    if server.team_diff(team1, team2, with_specs, with_bots) > diff
    then
        return true
    end
    
    return
end

function server.two_teams_unbalanced(team1, team2, with_specs, with_bots)

    if not team1 or not team2
    then
	team1 = "good"
	team2 = "evil"
    end
    
    return compare_two_teams_by_diff(1, team1, team2, with_specs, with_bots)
end

function server.two_teams_sizes_identical(team1, team2, with_specs, with_bots)

    if not team1 or not team2
    then
	team1 = "good"
	team2 = "evil"
    end
    
    return compare_two_team_by_diffs(0, team1, team2, with_specs, with_bots)
end

function server.teams_unbalanced(with_specs, with_bots)

    local sizes		= {}
    local checked	= {}
    
    local function get_size(team)
    
	if not sizes[team]
	then
	    sizes[team] = server.team_size(team, with_specs, with_bots)
	end
	
	return sizes[team]
    end
    
    for _, team in pairs(server.teams())
    do
	if not checked[team]
	then
	    checked[team] = {}
	end
	
	local size = get_size(team)
	
	for _, t in pairs(server.teams())
	do
	    if not checked[t]
	    then
		checked[t] = {}
	    end
	    
	    if not (team == t) and (not checked[team][t] or not checked[t][team]) and (math.abs(size - get_size(t)) > 1)
	    then
		return true
	    end
	    
	    checked[team][t] = true
	    checked[t][team] = true
	end
    end
    
    return
end

function server.team_sizes_identical(with_specs, with_bots)

    local sizes		= {}
    local checked	= {}
    
    local function get_size(team)
    
	if not sizes[team]
	then
	    sizes[team] = server.team_size(team, with_specs, with_bots)
	end
	
	return sizes[team]
    end
    
    for _, team in pairs(server.teams())
    do
	if not checked[team]
	then
	    checked[team] = {}
	end
	
	local size = get_size(team)
	
	for _, t in pairs(server.teams())
	do
	    if not checked[t]
	    then
		checked[t] = {}
	    end
	    
	    if not (team == t) and (not checked[team][t] or not checked[t][team]) and not (math.abs(size - get_size(t)) == 0)
	    then
		return
	    end
	    
	    checked[team][t] = true
	    checked[t][team] = true
	end
    end
    
    return true
end

-- will return a random choosen team, when more than 2 teams
-- returns nil when less than 2 teams
function server.other_team(team)

    if server.teams_count() < 1
    then
	return
    end
    
    local t
    
    for _, t in pairs(server.teams())
    do
	if not (team == t)
	then
	    return t
	end
    end
end
