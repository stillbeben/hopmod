-- #changetime <time>

return function(cn, time)

    time = tonumber(time)
    
    if time >= 0 and time < 13670 then
        server.changetime(((time*60)*1000))
    else
        server.player_msg(cn,red("max map.time is 13669"))
    end
    
end
