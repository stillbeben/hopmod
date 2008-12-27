#!/usr/bin/perl

use LWP::UserAgent;
use HTTP::Request;
use warnings;
use strict;
use POE qw(Component::IRC::State Component::IRC::Plugin::AutoJoin Component::IRC::Plugin::Connector Component::IRC::Plugin::FollowTail);
use Config::Auto;
use vars qw($cn $config $irc @playerstats @serverlist @data $rrd $topriv $version $hopvar $rev @split);

$config = Config::Auto::parse("../conf/vars.conf" , format => "equal");

$version = "1.21"; #<----Do NOT change this or I will kill you
print "<Starting HopBot V$version support at #hopmod\@irc.gamesurge.net> \n";

&toserverpipe("irc_pid = $$"); #Send the server my pid for restarting purposes.

if ( $config->{irc_player_locator} eq "1" ) {
        if ( eval { require 'bin/player_locator.pm' } ) {
                print "INFO     : Player Locator module is    ENABLED\n";
        } else {
                print "ERROR    : Trending module is    DISABLED\n\n";
		print "ERROR MSG: $@\n";
        }
}

my ($irc) = POE::Component::IRC::State->spawn();
POE::Session->create(
	inline_states => {
		_start     => \&bot_start,
		irc_001    => \&on_connect,
		irc_public => \&on_public,
		irc_msg    => \&on_private,
		irc_whois  => \&on_whois,
		irc_tail_input	=> \&on_tail_input,
		irc_error	=> \&on_error,
		irc_socketerr	=> \&on_disconnect,
	},
	package_states => [ 
	main => [ qw(bot_start lag_o_meter ) ],
	],
	);

sub bot_start {
	my $kernel  = $_[KERNEL];
	my $heap    = $_[HEAP];
	my $session = $_[SESSION];
	my %channels = ( $config->{irc_channel}   => '' ); 
	$heap->{connector} = POE::Component::IRC::Plugin::Connector->new();
	$irc->plugin_add( 'FollowTail' => POE::Component::IRC::Plugin::FollowTail->new( filename => $config->{irc_serverlogfile},));
	$irc->plugin_add('AutoJoin', POE::Component::IRC::Plugin::AutoJoin->new( Channels => \%channels, RejoinOnKick => 1));
	$irc->plugin_add( 'Connector' => $heap->{connector} );
	$irc->yield( register => "all" );
	my $nick = $config->{irc_botname};
	$irc->yield( connect =>
		{ Nick => $nick,
			Username => $config->{irc_username},
			Ircname  => $config->{irc_username},
			Server   => $config->{irc_network},
			Port     => $config->{irc_port},
			Debug	 => $config->{irc_debug},
		}
		);
	$kernel->delay( 'lag_o_meter' => 60 );
}
sub lag_o_meter {
	my ($kernel,$heap) = @_[KERNEL,HEAP];
	$kernel->delay( 'lag_o_meter' => 60 );
	return;
}
sub on_connect {
	my ( $kernel, $sender, $message, $message2 ) = @_[ KERNEL, SENDER, ARG0, ARG1 ];
	my $ts = scalar localtime;
	print "CONNECT	: [$ts] Connected to $message\n";
	print "WELCOME	: [$ts] $message2\n";
	$irc->yield( join => $config->{irc_channel} );
	$irc->yield( join => $config->{irc_monitor_channel} );
}
sub on_public {
	my ( $kernel, $sender, $who, $where, $msg ) = @_[ KERNEL, SENDER, ARG0, ARG1, ARG2 ];
	my $nick = ( split /!/, $who )[0];
	my $channel = $where->[0];
	my $ts = scalar localtime;
	if ( $config->{irc_channel} eq "1" ) { print "[$ts] <$nick:$channel> $msg\n"; }
	&process_command($sender, $who, $msg, $channel);
}
sub on_private {
	my ( $kernel, $sender, $who, $where, $msg ) = @_[ KERNEL, SENDER, ARG0, ARG1, ARG2 ];
	my $nick = ( split /!/, $who )[0];
	my $channel = $where->[0];
	my $ts = scalar localtime;
	print "INFO	: [$ts] <$nick:$channel> $msg\n";
	&process_command($sender, $nick, $msg, $nick);
}
sub on_error {
	my ( $kernel, $sender, $error ) = @_[ KERNEL, SENDER, ARG0 ];
	my $ts = scalar localtime;
	print "ERROR	: [$ts] ERROR $error\n";

}
sub on_disconnect {
        my ( $kernel, $sender, $error ) = @_[ KERNEL, SENDER, ARG0 ];
        my $ts = scalar localtime;
        print "DISCONNECT	: [$ts] ERROR $error\n";

}
sub on_tail_input {
	my ($kernel, $sender, $filename, $input) = @_[KERNEL, SENDER, ARG0, ARG1];
        $irc->yield( privmsg => $config->{irc_channel} => &filterlog($_[ARG1]) );
	&toserverpipe("irc_pid = $$"); # Regular refresh for safetys sake
}
sub process_command {
	my $sender = shift;
	my $who = shift;
	my $command = shift;
	my $channel = shift;
	my $nick = ( split /!/, $who )[0];
    	my $poco_object = $sender->get_heap();
	if ( $command =~ /^$config->{irc_botcommandname}/i  )                            {
		if ( ! $poco_object->is_channel_operator( $config->{irc_channel}, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
		##### COMMAND PROCESSING #####

               ##### FIND #####
                if ( $command =~ / find *(.*)/i )
                { $topriv = $nick;&find('1',$1,$channel);
                ; return }
		##### SAY #####
		if ( $command =~ / say (.*)/i )
                { 
			&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/SAY/-\x03 Console($nick): \x034$1\x03"); &toserverpipe("console [$nick] [$1]");
		&toirccommandlog("Console($nick): $1"); print "Console($nick): $1"; return }
		##### KICK #####
		if ( $command =~ / kick ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/KICK/-\x03 $nick kicked $1"); &toserverpipe("kick $1"); 
		&toirccommandlog("$nick KICKED $1"); return }
		##### BAN #####
                if ( $command =~ / ban ([0-9]+)/i )
                { &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/BAN/-\x03 $nick banned $1 until server reboot"); &toserverpipe("kick $1 44640");
                &toirccommandlog("$nick BANNED $1"); return }
		##### CLEARBANS #####
		if ( $command =~ / clearbans/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/CLEARBANS/-\x03 $nick has cleared bans"); &toserverpipe("clearbans"); 
		&toirccommandlog("$nick CLEARED BANS"); return }
		##### SPECTATOR ######
		if ( $command =~ / spec.*\s([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/SPEC/-\x03 $nick has spec'd $1"); &toserverpipe("spec $1"); 
		&toirccommandlog("$nick has SPECTATED $1"); return }
		##### UNSPECTATOR ######
		if ( $command =~ / unspec.*\s([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/UNSPEC/-\x03 $nick has unspec'd $1"); &toserverpipe("unspec $1"); 
		&toirccommandlog("$nick has UNSPECTATED $1 "); return }
		##### MUTE #####
		if ( $command =~ / mute ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/MUTE/-\x03 $nick muted $1"); &toserverpipe("player_var $1 mute 1");
		&toirccommandlog("$nick MUTED $1"); return }
		##### UNMUTE #####
		if ( $command =~ / unmute ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/UNMUTE/-\x03 $nick Unmuted $1"); &toserverpipe("player_var $1 mute 0"); 
		&toirccommandlog("$nick UNMUTED $1"); return }
		##### GIVEMASTER #####
		if ( $command =~ / givemaster ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/GIVEMASTER/-\x03 $nick gave master to $1"); &toserverpipe("setmaster $1 1");
		&toirccommandlog("$nick GIVEMASTER $1"); return }
		##### TAKEMASTER #####
		if ( $command =~ / takemaster ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/TAKEMASTER/-\x03 $nick took master from $1"); &toserverpipe("setmaster $1 0"); 
		&toirccommandlog("$nick TAKEMASTER $1"); return }
		##### GIVEINVMASTER #####
		if ( $command =~ / giveinvmaster ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/GIVEINVMASTER/-\x03 $nick gave invisible master to $1"); &toserverpipe("setpriv $1 master");
		&toirccommandlog("$nick GIVEINVMASTER $1"); return }
		##### TAKEINVMASTER #####
		if ( $command =~ / takeinvmaster ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/TAKEINVMASTER/-\x03 $nick took invisible master from $1"); &toserverpipe("setpriv $1 none"); 
		&toirccommandlog("$nick TAKEINVMASTER $1"); return }
		##### MASTERMODE #####
		if ( $command =~ / mastermode ([0-9]+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/MASTERMODE/-\x03 $nick changed mastermode to $1"); &toserverpipe("mastermode $1"); 
		&toirccommandlog("$nick MASTERMODE $1"); return }
		##### MAPCHANGE #####
		if ( $command =~ / map (\S+) (\S+)/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/MAPCHANGE/-\x03 $nick changed map to \x037$1\x03 on \x037$2\x03"); &toserverpipe("changemap $1 $2"); 
		&toirccommandlog("$nick MAP $1 $2"); return }
		##### HELP #####
		if ( $command =~ / help/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/HELP/-\x03 help can be found here http://hopmod.e-topic.info/index.php5?title=IRC_Bot"); return}
		##### WHO #####
		if ( $command =~ / who/i )
		{ $topriv = $nick ; my $line = &toserverpipe("who");
                my $temp = "";
                my @temp = split (/ /, $line);
                foreach (@temp) {
                        if ( $_ =~ /N=/ ) {
                                $temp = $_;
                                $temp =~ s/N=(\S*)C=(\S*)P=(\S*)/\x0312$1\x03\($2\)\x03  /;
                                push (@split, $temp);
                        } else { if ($line eq "0") {&sendtoirc($channel,"Apparently no one is connected"); return} }

                }
	        &splittoirc($channel,"\x03\x036IRC\x03         \x034-/WHO/-\x03 is"); return}
		##### DIE #####
		if ( $command =~ / die/i ) 
		{&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/DIE/-\x03 $nick terminated the bot") ;
		&toserverpipe("restart_ircbot") ; 
		&toirccommandlog("$nick RESTART_IRCBOT"); return }
		##### VERSION #####
		if ( $command =~ / version/i )
                { my @info = `svn info`;
		foreach (@info) { if ( $_ =~ /Revision: (.*)/ ) { $rev = $1 }}
		&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/VERSION/-\x03 HopBot\x034V $version\x03 HopMod Rev:\x034 $rev\x03"); return}
		##### RESTART SERVER #####
		if ( $command =~ / restart server/i )
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/RESTART SAUER/-\x03 $nick restarted the server process"); &toserverpipe("restarter; shutdown");
		&toirccommandlog("$nick RESTART_SERVER"); return }
		##### SHOWALIAS #####
                if ( $command =~ / showalias ([0-9]+.*)/i )
                { my $line = &toserverpipe("showaliases $1"); $topriv = $nick; 
		&toirccommandlog("$nick SHOWALIAS $1"); 
        	&sendtoirc($channel,"\x036IRC\x03         \x034-/SHOWALIAS/-\x03 \x037$line\x03 @") ; return}
		##### SCORE #####
		if ( $command =~ / score/i )
		{ $topriv = $nick ; my $line = &toserverpipe("score");
		&toirccommandlog("$nick SCORE"); 
                my $temp = "";
                my @temp = split (/ /, $line);
                foreach (@temp) {
                        if ( $_ =~ /N=/ ) {
                                $temp = $_;
                                $temp =~ s/N=(\S*)F=(\S*)D=(\S*)/\x0312$1\x03\[\x033 $2\x03 \/\x034 $3 \x03\]  /;
                                push (@split, $temp);
                        } else { if ($line eq "0") {&sendtoirc($channel,"Apparently no one is connected"); return} }
                }
       		&splittoirc($channel,"\x03\x036SCORE\x03 [\x033kills\x03/\x034deaths\x03]"); return }
		##### SETMOTD #####
		if ( $command =~ / setmotd (.*)/i)
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/MOTD/-\x03 $nick changed to \x037$1\x03");
		&toserverpipe("motd = \"$1\""); 
		&toirccommandlog("$nick SETMOTD $1"); return }
		##### GETMOTD #####
                if ( $command =~ / getmotd/i )
		{
                my $line = &toserverpipe("getvar motd"); 
		&sendtoirc($channel,"$line");
		&toirccommandlog("$nick GETMOTD"); return }
		##### GETSLOTS #####
                if ( $command =~ / getslots/i )
		{my $line = &toserverpipe("getvar maxclients"); 
		&toirccommandlog("$nick GETSLOTS"); 
		&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/SLOTS/-\x03 are set to \x037$line\x03");return }
		##### SETSLOTS #####
		if ( $command =~ / setslots (.*)/i)
		{ &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/SLOTS/-\x03 $nick changed to \x037$1\x03");
		&toserverpipe("maxclients $1"); 
		&toirccommandlog("$nick SETSLOTS $1"); return }
		##### GETVAR #####
		if ( $command =~ / getvar (.*)/i )
                { $hopvar = $1; if ( $hopvar eq "irc_adminpw" ) { $hopvar = "title" }
		my $output = &toserverpipe("getvar $hopvar");
		&toirccommandlog("$nick GETVAR $hopvar"); 
		&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/GETVAR $1/-\x03 is\x037 $output\03"); return }
		##### STATUS #####
		if ( $command =~ / status/i )
                { my $line = &toserverpipe("status");
		&toirccommandlog("$nick STATUS"); 
      		if ($line =~ /COMMAND STATUS map=(\S*) mode=(.*) time=(\S*) master=(\S*) mm=(\S*) playercount=(\S*) avgping=(\S*)/)
        	{&sendtoirc($channel,"\x03\x036IRC\x03         \x034-/STATUS/-\x03 Map\x037 $1\x03 Mode\x037 $2\x03 Timeleft\x037 $3\x03 Master\x037 $4\x03 Mastermode\x037 $5\x03 Playercount\x037 $6\x03 Avgping\x037 $7\x03"); } return;}
		#######################################PASSWORD PROTECTED COMMANDS
		##### UPDATE #####	
		if ( $command =~ / update $config->{irc_adminpw}/i )
                { &toserverpipe("updating"); &sendtoirc($channel,&update);
                &toirccommandlog("$nick UPDATE"); return }
		##### CUBESCRIPT ##### 
		if ( $command =~ / cubescript (.*) $config->{irc_adminpw}/i )
                { my $output = &toserverpipe($1); &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/CUBESCRIPT/-\x03 $nick executed \x037$1\x03:$output"); 
                &toirccommandlog("$nick CUBESCRIPT $1"); return }
		#######Catchall
		if ( $command =~ /^$config->{irc_botcommandname} (.*)/i )
		{ my $output = &toserverpipe("irc_$1"); &sendtoirc($channel,"\x03\x036IRC\x03         \x034-/OUTPUT/-\x03 $nick executed \x037$1\x03:$output"); } return;
	}
}

sub filterlog {
	my $line = shift;
	##### ANNOUNCE #####
        if ($line =~ / ANNOUNCE (\S*) #announce (.*)/)
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
	if (  $line !~ /#announce/ ) {
		if ($line =~  /(\S*\([0-9]+\))(\(*.*\)*): (.*)/) 
		{ return "\x033CHAT\x03       \x0312$1\x034$2\x03 --> \x033$3\x03" } 
	} return; 
	##### MAP CHANGE #####
	if ($line =~ /new game: (.*), (.*), (.*)/) 
	{ return "\x032NEWMAP\x03     New map \x037$3\x03 for\x037 $2\x03 with\x037 $1\x03 " }
	##### MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) claimed master/) 
	{ return "\x034MASTER\x03     \x0312$1\x03 took master." }
	##### RELEASE MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) relinquished privileged status./) 
	{ return "\x034UNMASTADM\x03   \x0312$1\x03 relinquished privileged status" }
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


##########################################Command Filtering
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
sub toprivateirc {
	my $send = shift; 
	$irc->yield( privmsg => $topriv => "\x034$send\x03" );
}
sub sendtoirc {
	my $channel = shift;
	my $send = shift; 
	$irc->yield( privmsg => $channel => "\x034$send\x03" );
}
sub splittoirc {
	my $channel = shift;
	my $prefix = shift;
	my $arrLen = scalar @split; 
	my $temp = "";
	my $j = 0;
	my $temp2 = "";
	foreach (@split) {
		$temp2 = "$temp2 " . $_;
		$j++;
		if ( $j == 7 ) { if ( $arrLen > 14 ) {&toprivateirc("$prefix $temp2") } else { &sendtoirc($channel,"$prefix $temp2") } ; $temp2 ="" ; $j = 0 }
	}
	if ( $temp2 ne "" ) { if ( $arrLen > 14 ) {&toprivateirc("$prefix $temp2") } else { &sendtoirc($channel,"$prefix $temp2") }}
	undef @split; return  	
}
sub update {
	my $output = `svn update bin/irc.pl scripts/irc.csl`;
	&toserverpipe("exec scripts/irc.csl");
	return "\x03\x036IRC\x03         \x034-/BOT_UPDATE/-\x03 \x037$output\03";

}
sub toserverpipe {
	my $content = shift;
        my $connection = LWP::UserAgent->new();
        my $post = HTTP::Request->new(POST => "http://127.0.0.1:7894/serverexec");
        $post->content_type("text/cubescript");
        $post->content($content);
        my $response = $connection->request($post);
        return $response->content;
}
sub toirccommandlog {
        my $send = shift;
        my $ts = scalar localtime;
        open (FILE, '>>', $config->{irc_commandlog} ) or warn $!;
        print FILE ("[$ts] $send\n");
        close (FILE);
}

$poe_kernel->run();
exit 0;
