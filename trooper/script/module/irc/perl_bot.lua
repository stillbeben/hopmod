
os.execute("bin/server start_ircbot")

server.event_handler("shutdown", function() 
    server.stop_ircbot()
end)
