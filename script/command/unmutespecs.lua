
return function()
    
    if not server.unmute_spectators then
        return false, "mute module not loaded"
    end
    
    server.unmute_spectators()
end
