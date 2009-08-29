
local domain_name = server.name_reservation_domain
local domain_id = auth.get_domain_id(domain_name)
if not domain_id then error(string.format("name reservation failure: auth domain '%s' not found",domain_name)) end
local is_local = auth.is_domain_local(domain_id)

local db,perr,get_tag_id_of_tag_front,get_tag_id_of_tag_back,get_tag_id_of_tag_both,get_clan_id,get_tag_place

if server.use_tag_reservation == 1 then
    require "sqlite3"
    
    dofile("./script/db/sqliteutils.lua")
    db = sqlite3.open(server.auth_db_filename)
    
    get_tag_id_of_tag_front,perr = db:prepare("SELECT * FROM tags WHERE place = 1 AND tag_front = :tag")
    if not get_tag_id_of_tag_front then
        error(perr)
    end
    
    get_tag_id_of_tag_back,perr = db:prepare("SELECT * FROM tags WHERE place = 0 AND tag_back = :tag")
    if not get_tag_id_of_tag_back then
        error(perr)
    end
    
    get_tag_id_of_tag_both,perr = db:prepare("SELECT * FROM tags WHERE place = 2 AND tag_front = :tag_front AND tag_back = :tag_back")
    if not get_tag_id_of_tag_both then
        error(perr)
    end
    
    get_clan_id,perr = db:prepare("SELECT * FROM users WHERE domain_id = :domain_id AND name = :name")
    if not get_clan_id then
        error(perr)
    end
end

auth.add_domain_handler(domain_name, function(cn, name)
    
    -- Add user from remote domain. For this situation to arise the same domain
    -- has to be used by another system (i.e. stats auth) to send an auth request.
    if not is_local and not auth.found_name(name, domain_id) then
        auth.add_user(name, "", domain_name)
    end
    
    local pvars = server.player_pvars(cn)
    local vars = server.player_vars(cn)
    
    if pvars.nameprotect_wanted_authname and pvars.nameprotect_authname and pvars.nameprotect_wanted_authname ~= name then
	server.player_msg(cn,"You authenticated as " .. green(name) .. " but the " .. red("server was expecting you to auth as ") .. green(pvars.nameprotect_authname)  .. ". You have about " .. orange("10 seconds to rename"))
        return
    end
    
    pvars.name_verified = true
    pvars.reserved_name = name
    pvars.reserved_name_expire = tonumber(server.uptime) + tonumber(server.reserved_name_expire)
    
    vars.name_verified = true
    vars.reserved_name = name
    vars.reserved_name_expire = tonumber(server.uptime) + tonumber(server.reserved_name_expire)
    
    pvars.auth_name = name
end)

local function isPlayerVerified(cn)
    local pvars = server.player_pvars(cn)
    return pvars.name_verified and pvars.reserved_name == server.player_pvars(cn).nameprotect_authname and tonumber(server.uptime) < pvars.reserved_name_expire
end

local function getTagIdOfTagFront(tag)
    get_tag_id_of_tag_front:bind{tag = tag}
    local tmp = get_tag_id_of_tag_front:first_row()
    if not tmp then
        return
    end
    return tmp.id
end

local function getTagIdOfTagBack(tag)
    get_tag_id_of_tag_back:bind{tag = tag}
    local tmp = get_tag_id_of_tag_back:first_row()
    if not tmp then
        return
    end
    return tmp.id
end

local function getTagIdOfTagBoth(tag_front,tag_back)
    get_tag_id_of_tag_both:bind{tag_front = tag_front,tag_back = tag_back}
    local tmp = get_tag_id_of_tag_both:first_row()
    if not tmp then
        return
    end
    return tmp.id
end

local function nameInClan(name,domain_id,tag_id)
    get_clan_id:bind{domain_id = domain_id, name = name}
    local tmp = get_clan_id:first_row()
    if not tmp then
        return false
    elseif tonumber(tmp.clan_id) == tonumber(tag_id) then
        return true
    else
        return false
    end
end

local function checkPlayerName(cn)
    local function check_name(cn)
	auth.sendauthreq(cn, domain_name)
	local sid = server.player_sessionid(cn)
        local pid = server.player_id(cn)
        server.player_msg(cn,"You are using a " .. red("reserved name") .. " and have about " .. orange("10 seconds to authenticate") .. " your player name.")
        server.sleep(13000, function()
            if sid ~= server.player_sessionid(cn) or pid ~= server.player_id(cn) then
                return
            end
            if not isPlayerVerified(cn) then
                server.kick(cn, 0, "server", "using reserved name")
            end
        end)
    end
    
    local function check_tag(cn,playername)
        local sid = server.player_sessionid(cn)
	local pid = server.player_id(cn)
        server.player_msg(cn,"You are using a " .. red("reserved tag") .. " and have about " .. orange("10 seconds to rename"))
        server.sleep(13000, function()
            if sid ~= server.player_sessionid(cn) or pid ~= server.player_id(cn) then
        	return
	    end
            if server.player_name(cn) == playername then
                server.kick(cn, 0, "server", "using reserved tag")
            end
        end)
    end
    
    local playername = server.player_name(cn)
    
    if auth.found_name(playername,domain_id) and not isPlayerVerified(cn) then
        server.player_pvars(cn).nameprotect_wanted_authname = playername
        server.player_pvars(cn).nameprotect_authname = playername
        check_name(cn)
    elseif server.use_tag_reservation == 1 then
        local name_length = #playername
        for tag_front in db:cols("SELECT tag_front FROM tags WHERE place = 1") do
            if string.find(playername,tag_front) then
                local tag_id = getTagIdOfTagFront(tag_front)
                local tag_front_length = #tag_front
                if string.sub(playername,1,tag_front_length) == tag_front then
                    local lplayername = string.sub(playername,(tag_front_length + 1),name_length)
                    if ( not isPlayerVerified(cn) or ( isPlayerVerified(cn) and server.player_pvars(cn).reserved_name ~= lplayername )) and not nameInClan(lplayername,domain_id,tag_id) then
                        check_tag(cn,playername)
                        return
		    elseif not isPlayerVerified(cn) or server.player_pvars(cn).reserved_name ~= lplayername then
                        server.player_pvars(cn).nameprotect_wanted_authname = lplayername
                        server.player_pvars(cn).nameprotect_authname = lplayername
                        check_name(cn)
                        return
                    end
                end
            end
        end
        for tag_back in db:cols("SELECT tag_back FROM tags WHERE place = 0") do
            if string.find(playername,tag_back) then
		local tag_id = getTagIdOfTagBack(tag_back)
                local tag_back_length = #tag_back
		if string.sub(playername,((name_length - tag_back_length) + 1),name_length) == tag_back then
                    local lplayername = string.sub(playername,1,(name_length - tag_back_length))
                    if ( not isPlayerVerified(cn) or ( isPlayerVerified(cn) and server.player_pvars(cn).reserved_name ~= lplayername )) and not nameInClan(lplayername,domain_id,tag_id) then
                        check_tag(cn,playername)
                	return
                    elseif not isPlayerVerified(cn) or server.player_pvars(cn).reserved_name ~= lplayername then
                        server.player_pvars(cn).nameprotect_wanted_authname = lplayername
                        server.player_pvars(cn).nameprotect_authname = lplayername
                        check_name(cn)
                        return
                    end
                end
            end
        end
        for tag_front,tag_back in db:cols("SELECT tag_front,tag_back FROM tags WHERE place = 2") do
            if string.find(playername,tag_front) and string.find(playername,tag_back) then
                local tag_id = getTagIdOfTagBoth(tag_front,tag_back)
                local tag_front_length = #tag_front
                local tag_back_length = #tag_back
                if (string.sub(playername,1,tag_front_length) == tag_front) and (string.sub(playername,((name_length - tag_back_length) + 1),name_length) == tag_back) then
                    local tmp = string.sub(playername,(tag_front_length + 1),name_length)
                    local tmp_length = #tmp
                    local lplayername = string.sub(tmp,1,(tmp_length - tag_back_length))
                    if ( not isPlayerVerified(cn) or ( isPlayerVerified(cn) and server.player_pvars(cn).reserved_name ~= lplayername )) and not nameInClan(lplayername,domain_id,tag_id) then
                        check_tag(cn,playername)
                        return
                    elseif not isPlayerVerified(cn) or server.player_pvars(cn).reserved_name ~= lplayername then
                        server.player_pvars(cn).nameprotect_wanted_authname = lplayername
                        server.player_pvars(cn).nameprotect_authname = lplayername
                        check_name(cn)
                        return
                    end
                end
            end
        end
    end
end

server.event_handler("active", checkPlayerName)

server.event_handler("rename", function(cn)
    local pvars = server.player_pvars(cn)
    local vars = server.player_vars(cn)
    pvars.name_verified = false
    pvars.reserved_name = vars.reserved_name
    pvars.reserved_name_expire = vars.reserved_name_expire
    checkPlayerName(cn)
end)

-- #auth [<domain>]
if server.enable_auth_command == 1 then
    function server.playercmd_auth(cn,domain)
	local ldomain = domain_name
	if domain then
	    ldomain = domain
	end
        auth.sendauthreq(cn,ldomain)
    end
end

-- #showauth [<cn>]
if server.enable_showauth_command == 1 then
    function server.playercmd_showauth(showauth_cn,showauth_tcn)
	local lcn = showauth_cn
        if showauth_tcn then
    	    if not server.valid_cn(showauth_tcn) then
    		server.player_msg(showauth_cn,red("cn is not valid"))
    		return
    	    else
        	lcn = showauth_tcn
    	    end
        end
        local authname = server.player_pvars(lcn).auth_name
	if authname then
	    server.player_msg(showauth_cn,green(server.player_name(lcn)) .. magenta("(" .. lcn ..")") .. " is verified as " .. authname)
	end
    end
end

-- #shownonauths
if server.enable_shownonauths_command == 1 then
    function server.playercmd_shownonauths(shownonauths_cn)
        return mastercmd(function()
            for a,b in ipairs(server.players()) do
                if not server.player_pvars(b).auth_name then
                    server.player_msg(shownonauths_cn,green(server.player_name(b)) .. " " .. magenta("(" .. b ..")") .. " is not verified")
                end
            end
        end,shownonauths_cn)
    end
end

-- #showauths
if server.enable_showauths_command == 1 then
    function server.playercmd_showauths(showauths_cn)
        return mastercmd(function()
            for a,b in ipairs(server.players()) do
                if server.player_pvars(b).auth_name then
                    server.player_msg(showauths_cn,green(server.player_name(b)) .. magenta("(" .. b ..")") .. " is verified as " .. server.player_pvars(b).auth_name)
                end
            end
        end,showauths_cn)
    end
end
