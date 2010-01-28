    
local handler = server.event_handler("mapchange", function(map, mode)

    if mode == "coop edit" and server.allow_setmaster == false and server.disable_masterauth_in_coopedit == 1 then
        
        server.allow_setmaster = true
        
        local handler
        handler = server.event_handler("mapchange", function(map, mode)
            
            if mode ~= "coop edit" then
            
                server.allow_setmaster = false
                
                server.cancel_handler(handler)
                
                for index, cn in ipairs(server.players()) do
                    if server.player_priv_code(cn) == 1 then
                        server.unsetmaster(cn)
                    end
                end
            end
            
        end)
        
        server.msg("Master is now available to the first player who types /setmaster 1")
    end

end)

local function unload()
    server.cancel_handler(handler)
end

return {unload = unload}
