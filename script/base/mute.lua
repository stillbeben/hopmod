local DEFAULT_MUTE_TIME = server.default_mute_time or (1000 * 60 * 60)
local key_function = server.player_iplong
local muted = {}

function server.mute(cn, mute_time, reason)

    mute_time = mute_time or DEFAULT_MUTE_TIME
    
    local key = key_function(cn)
    
    muted[key] = true
    
    for _, cn in ipairs(server.players()) do 
        if key_function(cn) == key then
            local message = "You have been muted"
            if reason then
                message = message .. " because " .. reason
            end
            server.player_msg(cn, red(message))
        end
    end
    
    server.sleep(mute_time, function()
        muted[key] = nil
    end)
end

function server.unmute(cn)

    local key = key_function(cn)
    
    muted[key] = nil
    
    for _, cn in ipairs(server.clients()) do
		if key_function(cn) == key then
			server.player_msg(cn, "You have been unmuted.")
		end
    end
end

function server.is_muted(cn)
	return muted[key_function(cn)]
end

local function block_text(cn, text)

    local is_muted = muted[key_function(cn)]
    
    if is_muted then
        server.player_msg(cn, red("Your chat messages are being blocked."))
        return -1
    end
end

local text_event = server.event_handler("text", block_text)
local sayteam_event = server.event_handler("sayteam", block_text)

local function unload()

    server.mute = nil
    server.unmute = nil
end

return {unload = unload}
