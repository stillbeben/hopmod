#!/usr/bin/perl

package Filter;

sub Message { 
	my $line = shift;
	##### Returns EMPTY #####
	if ($line =~ /(#login|#changepw|#changeuserpw|#msg|#warning)/) { return }
	##### Bot Muting Functions #####
	if ($line =~ /COMMAND BOTMUTE (.*)/)
	{ sendtoirc($config->{irc_channel},"\x034BOTMUTE\x03       \x0312$1\x03 muted the IRC bot."); $mute_status = "1"; return;}
        if ($line =~ /COMMAND BOTUNMUTE (.*)/)
        { $mute_status = "0"; sendtoirc($config->{irc_channel},"\x039BOTUNMUTE\x03       \x0312$1\x03 unmuted the IRC bot."); return;}
	##### Invisible Master ##### 
	if ($line =~ /(\S*\([0-9]+\))(\(*.*\)*): #invadmin/)
	{ return "\x039MASTER\x03       \x0312$1\x03 attempted to take invisible master."}
	##### Cheater #####
	if ($line =~ /CHEATER: (.*) (.*)/)
	{ &sendtoirc($config->{irc_monitor_channel},"$1 reports: $2 is a Cheater, ADMINS please check this!"); return }
	##### 1on1 #####
		if ($line =~ /#1on1 (.*)/)
		{ return "\x039MATCH\x03       \x0312 The match starts now...\x03"}
		if ($line =~ /MATCH: (.*) versus (.*) on (.*)/)
		{ return "\x039MATCH\x03       \x034 $1 \x03 versus \x0312 $2 \x03 on \x037 $3 \x03"}
		if ($line =~ /MATCH: Game has end! (.*) wins, with (.*) - (.*) Poor (.*)/)
		{ return "\x039MATCH\x03       \x034 Game has ended! $1 \x03 wins, with $2 - $3 Poor $4 \x03"}
	##### PING #####
	if ($line =~ /PING: (.*) get kicked!/)
	{ return "\x039PING\x03    \x0312$1\x03 \x037was kicked for his high ping!\x03" }
	##### ANNOUNCE #####
	if ($line =~ /ANNOUNCE (\S*) (.*)/)
	{ &sendtoirc($config->{irc_monitor_channel},"$1 says $2"); return }
	##### CONNECT #####
	if ($line =~ /(\S*\([0-9]+\))(\(.+\))\((.*)\) connected/)
	{ return "\x039CONNECT\x03    \x0312$1\x03 \x037$3\x03" }
	##### DISCONNECT #####
	if ($line =~ /(\S*\([0-9]+\)) disconnected, (.+)/)
	{ return "\x032DISCONNECT\x03 \x0312$1\x03 \x037$2\x03" }
	##### RENAME #####
	if ($line =~ /(\S*\([0-9]+\)) renamed to (.+)/) 
	{ return  "\x032RENAME\x03     \x0312$1\x03 has renamed to \x037$2\x03"}
	##### CHAT #####
	if ($line =~  /(\S*\([0-9]+\))(\(*.*\)*): (.*)/) 
	{ return "\x033CHAT\x03       \x0312$1\x034$2\x03 --> \x033$3\x03" } 
	##### MAP CHANGE #####
	if ($line =~ /new game: (.*), (.*), (.*)/) 
	{ return "\x032NEWMAP\x03     New map \x037$3\x03 for\x037 $2\x03 with\x037 $1\x03 " }
	##### MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) claimed master/) 
	{ return "\x034MASTER\x03     \x0312$1\x03 took master." }
	##### RELEASE MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) relinquished admin./) 
	{ return "\x034NOADMIN\x03   \x0312$1\x03 relinquished admin" }
	##### KICK BAN #####
	if ($line =~ /(\S*) was kicked by (.*)/) 
	{ return "\x034KICK\x03      Master \x034$2\x03 kicked \x0312$1\x03" }
	##### KICK BAN 2
	if ($line =~ /(\S*\([0-9]+\)) kick\/banned for:(.+)\.\.\.by console./) 
	{ return "\x034KICK\x03      Console kicked \x0312$1\x03 for $2" }
	##### ADMIN #####
	if ($line =~ /(\S*\([0-9]+\)) claimed admin/) 
	{ return "\x034ADMIN\x03       \x0312$1\x03 took admin" }
	##### TEAM CHANGE
	if ($line =~ /(\S*\([0-9]+\)) changed team to (.+)/) 
	{ return "\x034CHANGETEAM \x03\x0312$1\x03 changed teams to \x037$2\x03" }
	##### MAP VOTE #####
	if ($line =~ /(\S*\([0-9]+\)) suggests (.+) on map (.+)/) 
	{ return "\x033SUGGEST\x03    \x0312$1\x03 suggests \x037$2\x03 on \x037$3\x03" }
	##### SERVER RESTART #####
	if ($line =~ /server started (.+)/) 
	{ return "\x034SERVER\x03    Server Restarted at\x037 $1\x03" }
	##### MASTERMODE #####
	if ($line =~ /mastermode is now ([0-9])/) 
	{ return "\x034MASTERMODE\x03  Mastermode is now\x0312 $1\x03" }
	##### IRC BOT SHUTDOWN #####
        if ($line =~  /Terminating the IRC bot/)
        { return "\x034SERVER\x03 \x0312IRC Bot Shutdown Initiated\x03" }
	##### SPECTATOR ###
	if ($line =~ /(\S*\([0-9]+\)) (.*) spectators/) 
	{ return "\x034SPECTATOR\x03  \x0312$1\x03 $2 spectators" }
	##### SERVER UPDATE #####
	if ($line =~  /(Performing server update:.*)/) 
	{ return "\x034SERVER\x03\x0312 $1\x03" }
	##### SERVER SHUTDOWN #####
	if ($line =~  /server shutdown (.*)/) 
	{ return "\x034SERVER\x03 \x0312Server Shutdown at $1\x03" }
	##### NEW COOP MAP #####
	if ($line =~  /(\S*\([0-9]+\)) set new map of size ([0-9]*)/) 
	{ return "\x034NEWCOOPMAP\x03 \x0312$1\x03 starts new map of size\x037 $2\x03" }
	##### APPROVE MASTER #####
	if ($line =~  /(\S*\([0-9]+\)) approved for master by (.+\([0-9]+\))/) 
	{ return "\x032APPROVE\x03    \x0312$1\x03 was approved for master by \x0312$2\x03" }	
	##### GENERIC #####
	if ($line =~  /Apparently no one is connected/)
        { return "Apparently no one is connected" }
	##### AUTHENTICATION #####
	if ($line =~  /(\S*\([0-9]+\)) passed authentication as '(.*)'./) 
	{ return "\x034AUTH\x03    \x0312$1\x03 passed authentication as \x037$2\x03" }
	
	return $line;
}
return 1;