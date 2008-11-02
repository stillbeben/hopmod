#!/usr/bin/env ruby

require 'net/http'
require 'readline'

begin
    Net::HTTP.version_1_2
    Net::HTTP.start("localhost",7894){|http|
        while true
            command = Readline::readline("server> ")
            if command.nil?
                puts "\n"
                exit
            end
            if command.length > 0
                Readline::HISTORY.push(command)
                
                req=Net::HTTP::Post.new("/serverexec")
                req.content_type=("text/cubescript")
                req.content_length = command.length()
                req.body = command
                
                res = http.request(req)
                puts res.read_body
            end
        end
    }
rescue
    puts "Error: #{$!}"
ensure
    
end
