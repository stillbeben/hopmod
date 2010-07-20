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
            return mode_aliases[mode] 
        else
            return mode
        end
    end
end

do
    local modes = {
        ["ffa"]                 = true,
        ["coop edit"]           = true,
        ["teamplay"]            = true,
        ["instagib"]            = true,
        ["instagib team"]       = true,
        ["efficiency"]          = true,
        ["efficiency team"]     = true,
        ["tactics"]             = true,
        ["tactics teams"]       = true,
        ["capture"]             = true,
        ["regen capture"]       = true,
        ["ctf"]                 = true,
        ["insta ctf"]           = true,
        ["protect"]             = true,
        ["insta protect"]       = true,
        ["hold"]                = true,
        ["insta hold"]          = true,
        ["efficiency ctf"]      = true,
        ["efficiency protect"]  = true,
        ["efficiency hold"]     = true
    }
        
    function server.valid_gamemode(input)
        return modes[input] == true
    end
end

