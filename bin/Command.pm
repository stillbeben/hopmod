#!/usr/bin/perl 

package Command;

sub Process {
	my $sender = shift;
	my $who = shift;
	my $command = shift;
	my $channel = shift;
	my $nick = ( split /!/, $who )[0];
		if ( $config->{irc_player_locator} eq "1" ) {
                	if ( $command =~ / find *(.*)/i )
	                { $topriv = $nick;&find('1',$1,$channel);
	                ; return }
		}

# Player Functions
		
		##### getteam #####
                if ( $command =~ / getteam ([0-9]+)/i )
                { my $team = main::toserverpipe("getteam $1"); my $name = main::toserverpipe("player_name $1"); return "CHANGETEAM","$name is on $team"}
		##### PAUSE #####
                if ( $command =~ / pause/i ) {
                my $output = main::toserverpipe("pausegame 1");
		return "PAUSE","$nick paused the game $output"}
		##### PAUSE #####
                if ( $command =~ / unpause/i )
                { my $output = main::toserverpipe("pausegame 0"); return "UNPAUSE","$nic unpaused the game $output"}
		##### SLAY #####
                if ( $command =~ / slay ([0-9]+)/i )
                { my $output = main::toserverpipe("player_slay $1"); return "SLAY","$nick slayed $1 $output"}
		##### PRIVSAY #####
                if ( $command =~ / privsay ([0-9]+) (.*)/i )
                { main::toserverpipe("player_msg $1 $2"); return "PRIVSAY","MSG($1): $2"}
                ##### CHANGETEAM #####
                if ( $command =~ / changeteam ([0-9]+) (.*)/i )
                { main::toserverpipe("changeteam $1 $2"); return "CHANGETEAM","$nick changed $1 to team $2"}
		##### KICK #####
                if ( $command =~ / kick ([0-9]+)/i )
                { main::toserverpipe("kick $1"); return "KICK","$nick kicked $1"}
                ##### SPECTATOR ######
                if ( $command =~ / spec.*\s([0-9]+)/i )
                { main::toserverpipe("spec $1"); return "SPEC","$nick spec'd $1"}
                ##### UNSPECTATOR ######
                if ( $command =~ / unspec.*\s([0-9]+)/i )
                { main::toserverpipe("unspec $1"); return "UNSPEC","$nick unspec'd $1"}
                ##### MUTE #####
                if ( $command =~ / mute ([0-9]+)/i )
                { main::toserverpipe("mute $1"); return "MUTE","$nick muted $1"}
                ##### UNMUTE #####
                if ( $command =~ / unmute ([0-9]+)/i )
		{ main::toserverpipe("unmute $1"); return "UNMUTE","$nick muted $1"}
                ##### SETMASTER #####
                if ( $command =~ / setmaster ([0-9]+)/i )
                { main::toserverpipe("setmaster $1 1"); return "GIVEMASTER","$nick gave master to $1"}
                ##### SETADMIN #####
                if ( $command =~ / setadmin ([0-9]+)/i )
                { main::toserverpipe("setadmin $1 1"); return "SETADMIN","$nick gave admin to $1"}
                ##### UNSETMASTER #####
                if ( $command =~ / unsetmaster/i )
                { main::toserverpipe("unsetmaster"); return "TAKEMASTER","$nick took master from $1"}
                ##### PLAYERS #####
                if ( $command =~ / players/i )
                { my $players = main::toserverpipe("players"); return "PLAYERS","Players: $players"}
                ##### SPECTATORS #####
                if ( $command =~ / spectators/i )
                { my $spectators = main::toserverpipe("spectators"); return "SPECTATORS","Spectators: $spectators"}
		##### RECORDDEMO #####
                if ( $command =~ / recorddemo (.*)/i )
                { my $output = main::toserverpipe("recorddemo $1"); return "RECORDDEMO","$nick has started demo recording as file $1 $output"}
		##### STOPDEMO #####
                if ( $command =~ / stopdemo/i )
                { my $output = main::toserverpipe("stopdemo"); return "STOPDEMO","$nick has stopped demo recording $output"}

# Team Functions

		if ( $command =~ / teams/i )
                { my $teams = main::toserverpipe("teams"); return "TEAMS","Teams: $teams"}


##### SAY #####
		if ( $command =~ / say (.*)/i )
		{ main::toserverpipe("console [$nick] [$1]");	return "SAY", "Console($nick): \x034$1\x03" }
##### CLEARBANS #####
		if ( $command =~ / clearbans/i )
		{ main::toserverpipe("clearbans");		return "CLEARBANS","$nick has cleared bans"}
		##### GIVEINVMASTER #####
		if ( $command =~ / giveinvmaster ([0-9]+)/i )
		{ main::toserverpipe("setpriv $1 master");	return "GIVEINVMASTER","$nick gave invisible master to $1"}
		##### TAKEINVMASTER #####
		if ( $command =~ / takeinvmaster ([0-9]+)/i )
		{ main::toserverpipe("setpriv $1 none");	return "TAKEINVMASTER","$nick took invisible master from $1"}
		##### MASTERMODE #####
		if ( $command =~ / mastermode ([0-9]+)/i )
		{ main::toserverpipe("mastermode $1");		return "MASTERMODE","$nick changed mastermode to $1"}
		##### MAPCHANGE #####
		if ( $command =~ / map (\S+) (\S+)/i )
		{ main::toserverpipe("changemap $2 $1");	return "MAP","$nick changed map to \x037$1\x03 on \x037$2\x03"}
		##### HELP #####
		if ( $command =~ / help/i )
		{ return "HELP","http://hopmod.e-topic.info/index.php5?title=IRC_Bot#Command_List"; }

                if ( $command =~ / who/i )
                { $topriv = $nick ; my $line = &main::toserverpipe("player_list");
                my $temp = "";
                my @temp = split (/ /, $line);
                foreach (@temp) {
                        if ( $_ =~ /N=/ ) {
                                $temp = $_;
                                $temp =~ s/N=(\S*)C=(\S*)P=(\S*)/\x0312$1\x03\($2\)\x03  /;
                                push (@main::split, $temp);
                        } else { if ($line eq "0") {&main::sendtoirc($channel,"Apparently no one is connected"); return } }

                }
		&main::splittoirc($channel,"\x03\x036IRC\x03         \x034-/WHO/-\x03 is"); return }



##### DIE #####
		if ( $command =~ / die/i ) 
		{ main::toserverpipe("restart_ircbot");		return "DIE","$nick terminated the bot"}
##### VERSION #####
		if ( $command =~ / version/i )
                { my @info = `svn info`; $version = $main::version; foreach (@info) { if ( $_ =~ /Revision: (.*)/ ) { $rev = $1 }}; return "VERSION", "HopBot\x034V $version\x03 HopMod Rev:\x034 $rev\x03"}
		##### RESTART SERVER #####
		if ( $command =~ / restart server/i )
		{ main::sendtoirc($channel,"\x03\x036IRC\x03         \x034-/RESTART SAUER/-\x03 $nick restarted the server process"); main::toserverpipe("restarter; shutdown");
		return }
##### SHOWALIAS #####
                if ( $command =~ / showaliases ([0-9]+.*)/i )
                { my $line = main::toserverpipe("showaliases $1"); $topriv = $nick; return "ALIASES","\x037$line\x03"}
		##### SCORE #####
                if ( $command =~ / score/i )
                { $topriv = $nick ; my $line = main::toserverpipe("score");
                my $temp = "";
                my @temp = split (/ /, $line);
		my @split;
                foreach (@temp) {
                        if ( $_ =~ /N=/ ) {
                                $temp = $_;
                                $temp =~ s/N=(\S*)F=(\S*)D=(\S*)/\x0312$1\x03\[\x033 $2\x03 \/\x034 $3 \x03\]  /;
                                push (@main::split, $temp);
                        } else { if ($line eq "0") {main::sendtoirc($channel,"Apparently no one is connected"); return} }
                }
                &main::splittoirc($channel,"\x03\x036SCORE\x03 [\x033kills\x03/\x034deaths\x03]"); return }
##### SETMOTD #####
		if ( $command =~ / setmotd (.*)/i)
		{ main::toserverpipe("motd = \"$1\""); 		return "SETMOTD","$nick changed to \x037$1\x03"; }
##### GETMOTD #####
                if ( $command =~ / getmotd/i )
		{ my $line = main::toserverpipe("getvar motd");	return "MOTD","Message of the day is: $line"} 
##### GETSLOTS #####
                if ( $command =~ / getslots/i )
		{ my $line = main::toserverpipe("getvar maxclients"); return "SLOTS","are set to \x037$line\x03" }
##### SETSLOTS #####
		if ( $command =~ / setslots (.*)/i)
		{ main::toserverpipe("maxclients $1"); 		return "IRC","$nick changed to \x037$1\x03"; }
		##### GETVAR #####
		if ( $command =~ / getvar (.*)/i )
                { $hopvar = $1; if ( $hopvar eq "irc_adminpw" ) { $hopvar = "title" } ; my $output = main::toserverpipe("getvar $hopvar"); 
		return "GETVAR", "$1/-\x03 is\x037 $output\03" }
		##### RELOAD #####
		if ( $command =~ / reload/i ) {
			if ($main::dynamic_reload == 1) {
				Module::Reload::Selective->reload(qw(Command));
				Module::Reload::Selective->reload(qw(Filter)); 	
				return "RELOAD", "Modules Reloaded Successfully"
			} else {
				return "ERROR", "The reload module did not load properly so this feature is inactive"
			}
		}
##### STATUS #####
		if ( $command =~ / status/i )
                { my $line = main::toserverpipe("status"); 
      		if ($line =~ /COMMAND STATUS map=(\S*) mode=(.*) time=(\S*) master=(\S*) mm=(\S*) playercount=(\S*) avgping=(\S*)/)
        	{return "STATUS","Map\x037 $1\x03 Mode\x037 $2\x03 Timeleft\x037 $3\x03 Master\x037 $4\x03 Mastermode\x037 $5\x03 Playercount\x037 $6\x03 Avgping\x037 $7\x03" } }
		#######Catchall
		if ( $command =~ /^$config->{irc_botcommandname} (.*)/i )
		{ my $output = main::toserverpipe("irc_$1"); 	return "OUTPUT","$nick executed \x037$1\x03:$output" } 
		return "HIGHLY ILLOGICAL","This command does not match any known to us"
}
return 1;
