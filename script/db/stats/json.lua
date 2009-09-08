
require "Json"

local function commit_game(game, players)

    local d = os.date("%0e%b%Y_%H:%M")
    local filename = string.format("log/game/%s_%s.json", d, game.map)
    
    local file = io.open(filename,"w");
    
    local root = {}
    root.game = game
    root.players = map_to_array(players)
    
    if server.gamemodeinfo.teams then
        
        root.teams = {}
        
        for i,teamname in ipairs(server.teams()) do
            
            team = {}
            team.name = teamname
            team.score = server.team_score(teamname)
            team.win = server.team_win(teamname)
            team.draw = server.team_draw(teamname)
            
            table.insert(root.teams, team)
        end
    end
    
    file:write(Json.Encode(root))
    file:flush()
end

return {commit_game = commit_game}
