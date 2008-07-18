#!/usr/bin/perl

use warnings;
use strict;
use POE qw(Component::IRC::State Component::IRC::Plugin::AutoJoin Component::IRC::Plugin::Connector Component::IRC::Plugin::FollowTail);
use Config::Auto;
use vars qw($topriv $master $config $version $zippy @zippy $hopvar $rev @split);
$config = Config::Auto::parse("../conf/vars.conf" , format => "equal");


#Module Processing
eval { require REST::Google::Translate } ;


$version = "1.15"; #<----Do NOT change this or I will kill you

#Config File Overrides
if ( defined $config->{irc_serverlogfile} ) {  }
if ( defined $config->{irc_commandlog} ) { }
if ( defined $config->{irc_serverpipe} ) {  }

print "Starting HopBot V$version by -={Pundit}=- #hopmod\@irc.gamesurge.net \n";

&toserverpipe("irc_pid = $$"); #Send the server my pid for restarting purposes.

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
	print " [$ts] Connected to $message\n";
	print " [$ts] $message2";
	$irc->yield( join => $config->{irc_channel} );
}
sub on_public {
	my ( $kernel, $sender, $who, $where, $msg ) = @_[ KERNEL, SENDER, ARG0, ARG1, ARG2 ];
	my $nick = ( split /!/, $who )[0];
	my $channel = $where->[0];
	my $ts = scalar localtime;
	print " [$ts] <$nick:$channel> $msg\n";
	&process_command($sender, $who, $msg);
}
sub on_private {
	my ( $kernel, $sender, $who, $where, $msg ) = @_[ KERNEL, SENDER, ARG0, ARG1, ARG2 ];
	my $nick = ( split /!/, $who )[0];
	my $channel = $where->[0];
	my $ts = scalar localtime;
	print " [$ts] <$nick:$channel> $msg\n";
	&process_command($sender, $nick, $msg);
}
sub on_error {
	my ( $kernel, $sender, $error ) = @_[ KERNEL, SENDER, ARG0 ];
	my $ts = scalar localtime;
	print " [$ts] ERROR $error\n";

}
sub on_disconnect {
        my ( $kernel, $sender, $error ) = @_[ KERNEL, SENDER, ARG0 ];
        my $ts = scalar localtime;
        print " [$ts] ERROR $error\n";

}
sub on_tail_input {
	my ($kernel, $sender, $filename, $input) = @_[KERNEL, SENDER, ARG0, ARG1];
        $irc->yield( privmsg => $config->{irc_channel} => &filterlog($_[ARG1]) );
}
sub process_command {
	my $sender = shift;
	my $who = shift;
	my $command = shift;
	my $channel = $config->{irc_channel};
	my $nick = ( split /!/, $who )[0];
    	my $poco_object = $sender->get_heap();
	if ( $command =~ /^$config->{irc_botcommandname}/i  )                            {
		if ( ! $poco_object->is_channel_operator( $channel, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
		##### COMMAND PROCESSING #####
		
		##### SAY #####
		if ( $command =~ / say (.*)/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={SAY}=-\x03 Console($nick): \x034$1\x03"); &toserverpipe("console [$nick] [$1]");
		&toirccommandlog("Console($nick): $1"); print "Console($nick): $1"; return }
		##### TRSAY #####
		if ( $command =~ / trsay (ja|en|de|it|fr|es|ru)\|(ja|en|de|it|fr|es|ru) (.+)/i & $config->{irc_trmodule} eq "1" ) {
                my $say = &translate($1,$2,$3);  
		&sendtoirc("\x03\x036IRC\x03         \x034-={SAY}=-\x03 Console($nick): \x034$say\x03");
                &toserverpipe("console [$nick] [$say]"); &toirccommandlog("Console($nick): $say"); print "Console($nick): $say"; return }
		##### TRANSLATE #####
		if ( $command =~ / translate (ja|en|de|it|fr)\|(ja|en|de|it|fr) (.+)/i & $config->{irc_trmodule} eq "1" )
                { my $tr = &translate($1,$2,$3); &sendtoirc("\x03\x036IRC\x03         \x034-={TRANSLATE}=-\x03 $tr"); return }
		##### KICK #####
		if ( $command =~ / kick ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={KICK}=-\x03 $nick kicked $1"); &toserverpipe("kick $1"); 
		&toirccommandlog("$nick KICKED $1"); return }
		##### CLEARBANS #####
		if ( $command =~ / clearbans/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034CLEARBANS\x03 $nick has cleared bans"); &toserverpipe("clearbans"); 
		&toirccommandlog("$nick CLEARED BANS"); return }
		##### SPECTATOR ######
		if ( $command =~ / spec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={SPEC}=-\x03 $nick has spec'd $1"); &toserverpipe("spec $1"); 
		&toirccommandlog("$nick has SPECTATED $1"); return }
		##### TICKLE ######
		if ( $command =~ /tickle.* $config->{irc_botcommandname}.*/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={WISDOM}=-\x03 $zippy[ rand scalar @zippy ]"); return }
		##### HI ######
		if ( $command =~ /hi.* $config->{irc_botcommandname}.*/i )
		{ &sendtoirc("hey $nick hows it going?");return }
		##### UNSPECTATOR ######
		if ( $command =~ / unspec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={UNSPEC}=-\x03 $nick has unspec'd $1"); &toserverpipe("unspec $1"); 
		&toirccommandlog("$nick has UNSPECTATED $1 "); return }
		##### MUTE #####
		if ( $command =~ / mute ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MUTE}=-\x03 $nick muted $1"); &toserverpipe("player_var $1 mute 1");
		&toirccommandlog("$nick MUTED $1"); return }
		##### UNMUTE #####
		if ( $command =~ / unmute ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={UNMUTE}=-\x03 $nick Unmuted $1"); &toserverpipe("player_var $1 mute 0"); 
		&toirccommandlog("$nick UNMUTED $1"); return }
		##### GIVEMASTER #####
		if ( $command =~ / givemaster ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={GIVEMASTER}=-\x03 $nick gave master to $1"); &toserverpipe("setmaster $1 1");
		&toirccommandlog("$nick GIVEMASTER $1"); return }
		##### TAKEMASTER #####
		if ( $command =~ / takemaster ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={TAKEMASTER}=-\x03 $nick took master from $1"); &toserverpipe("setmaster $1 0"); 
		&toirccommandlog("$nick TAKEMASTER $1"); return }
		##### MASTER #####
		if ( $command =~ / master$/i )
		{ &toserverpipe("masterwho") ;
		&toirccommandlog("$nick MASTERWHO $1"); return }
		##### GIVEINVMASTER #####
		if ( $command =~ / giveinvmaster ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={GIVEINVMASTER}=-\x03 $nick gave invisible master to $1"); &toserverpipe("setpriv $1 master");
		&toirccommandlog("$nick GIVEINVMASTER $1"); return }
		##### TAKEINVMASTER #####
		if ( $command =~ / takeinvmaster ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={TAKEINVMASTER}=-\x03 $nick took invisible master from $1"); &toserverpipe("setpriv $1 none"); 
		&toirccommandlog("$nick TAKEINVMASTER $1"); return }
		##### MASTERMODE #####
		if ( $command =~ / mastermode ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MASTERMODE}=-\x03 $nick changed mastermode to $1"); &toserverpipe("mastermode $1"); 
		&toirccommandlog("$nick MASTERMODE $1"); return }
		##### MAPCHANGE #####
		if ( $command =~ / map (\S+) (\S+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MAPCHANGE}=-\x03 $nick changed map to \x037$1\x03 on \x037$2\x03"); &toserverpipe("changemap $1 $2"); 
		&toirccommandlog("$nick MAP $1 $2"); return }
		##### HELP #####
		if ( $command =~ / help/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={HELP}=-\x03 help can be found here http://hopmod.e-topic.info/index.php5?title=IRC_Bot"); return}
		##### WHO #####
		if ( $command =~ / who\s*(.*)/i )
		{ $topriv = $nick ; &toserverpipe("who $1"); return}
		##### DIE #####
		if ( $command =~ / die/i ) 
		{&sendtoirc("\x03\x036IRC\x03         \x034-={DIE}=-\x03 $nick terminated the bot") ;
		&toserverpipe("restart_ircbot") ; 
		&toirccommandlog("$nick RESTART_IRCBOT"); return }
		##### VERSION #####
		if ( $command =~ / version/i )
                { my @info = `svn info`;
		foreach (@info) { if ( $_ =~ /Revision: (.*)/ ) { $rev = $1 }}
		&sendtoirc("\x03\x036IRC\x03         \x034-={VERSION}=-\x03 HopBot\x034V $version\x03 HopMod Rev:\x034 $rev\x03"); return}
		##### RESTART SERVER #####
		if ( $command =~ / restart server/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={RESTART SAUER}=-\x03 $nick restarted the server process"); &toserverpipe("restarter; shutdown");
		&toirccommandlog("$nick RESTART_SERVER"); return }
		##### SHOWALIAS #####
                if ( $command =~ / showalias ([0-9]+.*)/i )
                { &toserverpipe("showalias $1"); $topriv = $nick;
		&toirccommandlog("$nick SHOWALIAS $1"); return }
		##### SCORE #####
		if ( $command =~ / score/i )
		{ $topriv = $nick ; &toserverpipe("score");
		&toirccommandlog("$nick SCORE"); return }
		##### SETMOTD #####
		if ( $command =~ / setmotd (.*)/i)
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MOTD}=-\x03 $nick changed to \x037$1\x03");
		&toserverpipe("motd = \"$1\""); 
		&toirccommandlog("$nick SETMOTD $1"); return }
		##### GETMOTD #####
                if ( $command =~ / getmotd/i )
		{&sendtoirc("updating");
                &toserverpipe("getvar motd"); 
		&toirccommandlog("$nick GETMOTD"); return }
		##### GETSLOTS #####
                if ( $command =~ / getslots/i )
		{&toserverpipe("getvar maxclients"); 
		&toirccommandlog("$nick GETSLOTS"); return }
		##### SETSLOTS #####
		if ( $command =~ / setslots (.*)/i)
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={SLOTS}=-\x03 $nick changed to \x037$1\x03");
		&toserverpipe("maxclients $1"); 
		&toirccommandlog("$nick SETSLOTS $1"); return }
		##### GETVAR #####
		if ( $command =~ / getvar (.*)/i )
                { $hopvar = $1; if ( $hopvar eq "irc_adminpw" ) { $hopvar = "title" }
		&toserverpipe("getvar $hopvar");
		&toirccommandlog("$nick GETVAR $hopvar"); return }
		##### STATUS #####
		if ( $command =~ / status/i )
                { &toserverpipe("status");
		&toirccommandlog("$nick STATUS"); return }
		#######################################PASSWORD PROTECTED COMMANDS
		##### UPDATE #####	
		if ( $command =~ / update $config->{irc_adminpw}/i )
                { &toserverpipe("updating"); &update;
                &toirccommandlog("$nick UPDATE"); return }
		##### CUBESCRIPT ##### 
		if ( $command =~ / cubescript (.*) $config->{irc_adminpw}/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={CUBESCRIPT}=-\x03 $nick executed \x037$1\x03"); &toserverpipe($1);
                &toirccommandlog("$nick CUBESCRIPT $1"); return }
		
		##### CATCH ALL #####
		if ( $command =~ /^$config->{irc_botcommandname}/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={DOESNOTCOMPUTE}=-\x03 What the hell are you trying to tell me $nick");return }
		
		
	}
}

sub filterlog {
	my $line = shift;
	##### CONNECT #####
	if ($line =~ /(\S*\([0-9]+\))(\(.+\))\((.*)\) connected/)
	{ return "\x039CONNECT\x03    \x0312$1\x03 \x037$3\x03" }
	##### DISCONNECT #####
	if ($line =~ /(\S*\([0-9]+\)) disconnected, (.+)/)
	{ return "\x032DISCONNECT\x03 \x0312$1\x03 \x037$2\x03" }
	##### RENAME #####
	if ($line =~ /(\S*\([0-9]+\)) renamed to (.+)/) 
	{ return  "\x032RENAME\x03     \x0312$1\x03 has renamed to \x037$2\x03"}
	##### REGISTRATION
        if ($line =~  /#register/i) 
	{return "REGISTRATION";} # Filter Server Registration
	##### CHAT #####
	if ($line =~  /(\S*\([0-9]+\))(\(*.*\)*): (.*)/) 
	{ return "\x033CHAT\x03       \x0312$1\x034$2\x03 --> \x033$3\x03" }# Highlight game chat green
	##### MAP CHANGE #####
	if ($line =~ /new game: (.*), (.*), (.*)/) 
	{ return "\x032NEWMAP\x03     New map \x037$3\x03 for\x037 $2\x03 with\x037 $1\x03 " }
	##### MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) claimed master/) 
	{ $master = $1 ; return "\x034MASTER\x03     \x0312$1\x03 took master." }
	##### RELEASE MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) relinquished privileged status./) 
	{ $master = "NULL" ; return "\x034UNMASTADM\x03   \x0312$1\x03 relinquished privileged status" }
	##### KICK BAN #####
	if ($line =~ /(\S*) was kicked by (.*)/) 
	{ return "\x034KICK\x03      Master \x034$2\x03 kicked \x0312$1\x03" }
	##### KICK BAN 2
	if ($line =~ /(\S*\([0-9]+\)) kick\/banned for:(.+)\.\.\.by console./) 
	{ return "\x034KICK\x03      Console kicked \x0312$1\x03 for $2" }
	##### ADMIN #####
	if ($line =~ /(\S*\([0-9]+\)) claimed admin/) 
	{ $master = $1 ; return "\x034ADMIN\x03       \x0312$1\x03 took admin" }
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
	##### WHO #####
	if ($line =~ /COMMAND WHO/g) {
		my $temp = "";
		my @temp = split (/ /, $line);
		foreach (@temp) {
			if ( $_ =~ /N=/ ) { 
				$temp = $_;  
				$temp =~ s/N=(\S*)C=(\S*)P=(\S*)/\x0312$1\x03\($2\)\x03  /;
				push (@split, $temp);
			}
		}
	&splittoirc("\x03\x036IRC\x03         \x034-={WHO}=-\x03 is");
	return; 
	} 
		##### SCORE #####
	if ($line =~ /COMMAND SCORE/g) {
		my $temp = "";
		my @temp = split (/ /, $line);
		foreach (@temp) {
			if ( $_ =~ /N=/ ) { 
				$temp = $_;  
				$temp =~ s/N=(\S*)F=(\S*)D=(\S*)/\x0312$1\x03\[\x033 $2\x03 \/\x034 $3 \x03\]  /;
				push (@split, $temp);
			}
		}
	&splittoirc("\x03\x036SCORE\x03 [\x033kills\x03/\x034deaths\x03]");
	return;
	}
	##### GETVAR #####
        if ($line =~ /IRC .*-={GETVAR (.*)}=- is (.*)/)
        { return "\x03\x036IRC\x03         \x034-={GETVAR $1}=-\x03 is\x037 $2\03" }
	##### MASTER #####
        if ($line =~ /COMMAND CURRENTMASTER (.*)/)
        { return "\x03\x036IRC\x03         \x034-={MASTER}=-\x03 is \x037$1\03" }
	##### SHOWALAIS #####
	if ($line =~ /COMMAND SHOWALIAS N=(.*)A=(.*)/) 
	{ @split = split (/ /, $2); &splittoirc("\x036IRC\x03         \x034-={SHOWALIAS}=-\x03") ; return}
	##### SPECTATOR ###
	if ($line =~ /(\S*\([0-9]+\)) (.*) spectators/) 
	{ return "\x034SPECTATOR\x03  \x0312$1\x03 $2 spectators" }
	##### SERVER UPDATE #####
	if ($line =~  /(Performing server update:.*)/) 
	{ return "\x034SERVER\x03\x0312 $1\x03" }
	##### SERVER SHUTDOWN #####
	if ($line =~  /server shutdown (.*)/) 
	{ return "\x034SERVER\x03 \x0312Server Shutdown at $1\x03" }
	##### IRC BOT SHUTDOWN #####
	if ($line =~  /Terminating the IRC bot/) 
	{ return "\x034SERVER\x03 \x0312IRC Bot Shutdown Initiated\x03" }
	
	##### NEW COOP MAP #####
	if ($line =~  /(\S*\([0-9]+\)) set new map of size ([0-9]*)/) 
	{ return "\x034NEWCOOPMAP\x03 \x0312$1\x03 starts new map of size\x037 $2\x03" }
	##### APPROVE MASTER #####
	if ($line =~  /(\S*\([0-9]+\)) approved for master by (.+\([0-9]+\))/) 
	{ return "\x032APPROVE\x03    \x0312$1\x03 was approved for master by \x0312$2\x03" }	
	##### GENERIC #####
	if ($line =~  /Apparently no one is connected/)
        { return "Apparently no one is connected" }
	##### STATUS #####
        if ($line =~ /COMMAND STATUS map=(\S*) mode=(.*) time=(\S*) master=(\S*) mm=(\S*) playercount=(\S*)/)
        {return "\x03\x036IRC\x03         \x034-={STATUS}=-\x03 Map\x037 $1\x03 Mode\x037 $2\x03 Timeleft\x037 $3\x03 Master\x037 $4\x03 Mastermode\x037 $5\x03 Playercount\x037 $6\x03" }
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
	my $send = shift; 
	$irc->yield( privmsg => $config->{irc_channel} => "\x034$send\x03" );
}
sub splittoirc {
	my $prefix = shift;
	my $arrLen = scalar @split; 
	my $temp = "";
	my $j = 0;
	my $temp2 = "";
	foreach (@split) {
		$temp2 = "$temp2 " . $_;
		$j++;
		if ( $j == 7 ) { if ( $arrLen > 14 ) {&toprivateirc("$prefix $temp2") } else { &sendtoirc("$prefix $temp2") } ; $temp2 ="" ; $j = 0 }
	}
	if ( $temp2 ne "" ) { if ( $arrLen > 14 ) {&toprivateirc("$prefix $temp2") } else { &sendtoirc("$prefix $temp2") }}
	undef @split; return  	
}
sub update {
	my $output = `svn update bin/irc.pl scripts/irc.csl`;
	&toserverpipe("exec scripts/irc.csl");
	sendtoirc ("\x03\x036IRC\x03         \x034-={BOT_UPDATE}=-\x03 \x037$output\03");
	return;

}
sub toserverpipe {
        my $command = shift;
        open (FILE, '>', $config->{irc_serverpipe} ) or die $!; print FILE "$command\n"; close (FILE);
}
sub toirccommandlog {
        my $send = shift;
        my $ts = scalar localtime;
        open (FILE, '>>', $config->{irc_commandlog} ) or warn $!;
        print FILE ("[$ts] $send\n");
        close (FILE);
}
sub translate {
        my $langfrom = shift;
        my $langto = shift;
        my $text = shift;
        my $lang = "$langfrom|$langto";
        REST::Google::Translate->http_referer('http://example.com');
        my $res = REST::Google::Translate->new(
                q => $text,
                langpair => $lang
        );

        warn "response status failure" if $res->responseStatus != 200;

        my $translated = $res->responseData->translatedText;

	return $translated;
}
@zippy = (
	"Save the whales. Collect the whole set ",
	"A day without sunshine is like, night. ",
	"On the other hand, you have different fingers. ",
	"I just got lost in thought. It was unfamiliar territory. ",
	"42.7 percent of all statistics are made up on the spot. ",
	"99 percent of lawyers give the rest a bad name. ",
	"I feel like I.m diagonally parked in a parallel universe. ",
	"You have the right to remain silent. Anything you say will be misquoted, then used against you. ",
	"I wonder how much deeper the ocean would be without sponges ",
	"Honk if you love peace and quiet. ",
	"Remember half the people you know are below average. ",
	"Despite the cost of living, have you noticed how popular it remains? ",
	"Nothing is fool-proof to a talented fool. ",
	"He who laughs last thinks slowest. ",
	"Depression is merely anger without enthusiasm. ",
	"Eagles may soar, but weasels don.t get sucked into jet engines. ",
	"The early bird may get the worm, but the second mouse gets the cheese. ",
	"I drive way too fast to worry about cholesterol. ",
	"I intend to live forever - so far so good. ",
	"Borrow money from a pessimist - they don.t expect it back. ",
	"If Barbie is so popular, why do you have to buy her friends? ",
	"Quantum mechanics: The dreams stuff is made of. ",
	"The only substitute for good manners is fast reflexes. ",
	"Support bacteria - they.re the only culture some people have. ",
	"When everything is coming your way, you.re in the wrong lane and going the wrong way. ",
	"If at first you don.t succeed, destroy all evidence that you tried. ",
	"A conclusion is the place where you got tired of thinking. ",
	"Experience is something you don.t get until just after you need it. ",
	"For every action there is an equal and opposite criticism. ",
	"Bills travel through the mail at twice the speed of checks. ",
	"Never do card tricks for the group you play poker with. ",
	"No one is listening until you make a mistake. ",
	"Success always occurs in private and failure in full view. ",
	"The colder the x-ray table the more of your body is required on it. ",
	"The hardness of butter is directly proportional to the softness of the bread. ",
	"The severity of the itch is inversely proportional to the ability to reach it. ",
	"To steal ideas from one person is plagiarism; to steal from many is research. ",
	"To succeed in politics, it is often necessary to rise above your principles. ",
	"Monday is an awful way to spend 1/7th of your life. ",
	"Two wrongs are only the beginning. ",
	"The problem with the gene pool is that there is no lifeguard. ",
	"The sooner you fall behind the more time you.ll have to catch up. ",
	"A clear conscience is usually the sign of a bad memory. ",
	"Change is inevitable except from vending machines. ",
	"Plan to be spontaneous - tomorrow. ",
	"Always try to be modest and be proud of it! ",
	"If you think nobody cares, try missing a couple of payments. ",
	"How many of you believe in telekinesis? Raise my hand... ",
	"Love may be blind but marriage is a real eye-opener. ",
	"If at first you don.t succeed, then skydiving isn.t for you. ",
	"I am a traffic light, and Alan Ginsberg kidnapped my laundry in 1927!",
	"I'm a GENIUS!  I want to dispute sentence structure with SUSAN SONTAG!!",
	"Now I'm telling MISS PIGGY about MONEY MARKET FUNDS!",
	"I have a VISION!  It's a RANCID double-FISHWICH on an ENRICHED BUN!!",
	"My pants just went on a wild rampage through a Long Island Bowling Alley!!",
	"I always liked FLAG DAY!!",
	"I will establish the first SHOPPING MALL in NUTLEY, New Jersey...",
	"I used to be STUPID, too..before I started watching UHF-TV!!",
	"I smell like a wet reducing clinic on Columbus Day!",
	"Just walk along and try NOT to think about your INTESTINES being almost FORTY YARDS LONG!!",
	"It's the RINSE CYCLE!!  They've ALL IGNORED the RINSE CYCLE!!",
	"Yow!  It's some people inside the wall!  This is better than mopping!",
	"Is the EIGHTIES when they had ART DECO and GERALD McBOING-BOING lunch boxes??",
	"This PIZZA symbolizes my COMPLETE EMOTIONAL RECOVERY!!",
	"I call it a \"SARDINE ON WHEAT\"!",
	"Is it FUN to be a MIDGET?",
	"Someone in DAYTON, Ohio is selling USED CARPETS to a SERBO-CROATIAN!!",
	);
$poe_kernel->run();
exit 0;
