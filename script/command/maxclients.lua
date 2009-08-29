-- [[ Player command written by Thomas

function server.playercmd_maxclients(cn, arg1) 
    return admincmd(
	function ()
    	    if not arg1 then server.player_msg(cn, red("missing maxclient argument")) return end
    	    if arg1 >= server.playercount and arg1 <= "128" then
                server.maxplayers = arg1
            end
        end
    ,cn)
end

-- ]]
