#!/usr/bin/env ruby

require 'net/http'
require 'readline'

def execute_command(http, command)
    
    req = Net::HTTP::Post.new("/serverexec")
    req.content_type=("text/x-cubescript")
    req.content_length = command.length()
    req.body = command
    
    res = http.request(req)
    
    return res.read_body
end

def start_shell(hostname, port)
    
    Net::HTTP.version_1_2
    Net::HTTP.start(hostname, port){|http|
        
        puts "Connected to #{hostname}:#{port}"
        
        label = execute_command(http, "try [get shell_label] [result \"server\"]")
        
        while true
            
            command = Readline::readline("#{label}> ")
            if command.nil?
                puts "\n"
                exit
            end
            
            if command.length > 0
                Readline::HISTORY.push(command)
                puts execute_command(http, command)
            end
            
        end
    }

rescue EOFError
    puts "Error: lost connection with remote host."
    sleep 1
    puts "Attempting to reconnect..."
    start_shell(hostname, port)
    
rescue Errno::ECONNREFUSED
    puts "Error: connection refused."
    
rescue Errno::ENETUNREACH
    puts "Error: #{hostname} is unreachable."
    
rescue Errno::EINVAL
    puts "Error: invalid argument"
    
rescue IOError => e
    puts "Error: #{$!}"

rescue Interrupt
    puts ""
    
end

start_shell(ARGV[0] || "127.0.0.1", ARGV[1] || 7894)
