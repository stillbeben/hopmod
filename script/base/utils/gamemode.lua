-- Array of game mode names, used by the map rotation module
gamemodes = {
    [ 1] = "ffa",
    [ 2] = "coop edit",
    [ 3] = "teamplay",
    [ 4] = "instagib",
    [ 5] = "instagib team",
    [ 6] = "efficiency",
    [ 7] = "efficiency team",
    [ 8] = "tactics",
    [ 9] = "tactics team",
    [10] = "capture",
    [11] = "regen capture",
    [12] = "ctf",
    [13] = "insta ctf",
    [14] = "protect",
    [15] = "insta protect",
    [16] = "hold",
    [17] = "insta hold",
    [18] = "efficiency ctf",
    [19] = "efficiency protect",
    [20] = "efficiency hold"
}

do
    local mode_aliases = {
        ["instateam"]        = "instagib team",
        ["iteam"]            = "instagib team",
        ["insta"]            = "instagib",
        ["instactf"]         = "insta ctf",
        ["ictf"]             = "insta ctf",
        ["ctf"]              = "ctf",
        ["instaprotect"]     = "insta protect",
        ["iprotect"]         = "insta protect",
        ["efficteam"]        = "efficiency team",
        ["eteam"]            = "efficiency team",
        ["efficiency"]       = "efficiency",
        ["effic"]            = "efficiency",
        ["tacteam"]          = "tactics team",
        ["tteam"]            = "tactics team",
        ["tactics"]          = "tactics",
        ["tac"]              = "tactics",
        ["regencapture"]     = "regen capture",
        ["regencap"]         = "regen capture",
        ["regen"]            = "regen capture",
        ["cap"]              = "capture",
        ["coopedit"]         = "coop edit",
        ["coop"]             = "coop edit"
    }
    
    function server.parse_mode(mode)
        if not get_gamemode_info(mode) then
            return mode_aliaes[mode] 
        else
            return mode
        end
    end
end

