#!/usr/bin/perl

use warnings;
use strict;
use POE qw(Component::IRC::State Component::IRC::Plugin::AutoJoin Component::IRC::Plugin::Connector Component::IRC::Plugin::FollowTail);
use Switch;
use Config::Auto;
use vars qw($master $repeatcount $config $version $lastline $zippy @zippy @word );

$repeatcount = 1 ;
$version = "1.12"; #<----Do NOT change this or I will kill you




$config = Config::Auto::parse("../conf/vars.conf" , format => "equal");

#Config File Overrides
if ( defined $config->{irc_serverlogfile} ) {  }
if ( defined $config->{irc_commandlog} ) { }
if ( defined $config->{irc_serverpipe} ) {  }

print "Starting HopBot V$version by -={Pundit}=- #hopmod\@irc.gamesurge.net \n";

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
	if ( $command =~ /$config->{irc_botcommandname}/i  )                            {
		if ( ! $poco_object->is_channel_operator( $channel, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
		##### COMMAND PROCESSING #####
		
		##### SAY #####
		if ( $command =~ /$config->{irc_botcommandname}.* say (.*)/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={SAY}=-\x03 Console($nick): \x034$1\x03"); &toserverpipe("console $nick [$1]");
                &toirccommandlog("Console($nick): $1");  return }	
		##### KICK #####
		if ( $command =~ /$config->{irc_botcommandname}.* kick.* ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={KICK}=-\x03 $nick kicked $1"); &toserverpipe("kick $1"); 
		&toirccommandlog("$nick KICKED $1"); return }
		##### CLEARBANS #####
		if ( $command =~ /$config->{irc_botcommandname}.* clearbans/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034CLEARBANS\x03 $nick has cleared bans"); &toserverpipe("clearbans"); 
		&toirccommandlog("$nick CLEARED BANS"); return }
		##### SPECTATOR ######
		if ( $command =~ /$config->{irc_botcommandname}.* spec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={SPEC}=-\x03 $nick has spec'd $1"); &toserverpipe("spec $1"); 
		&toirccommandlog("$nick has SPECTATED $1"); return }
		##### TICKLE ######
		if ( $command =~ /tickle.* $config->{irc_botcommandname}.*/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={WISDOM}=-\x03 $zippy[ rand scalar @zippy ]"); return }
		##### HI ######
		if ( $command =~ /hi.* $config->{irc_botcommandname}.*/i )
		{ &sendtoirc("hey $nick hows it going?");return }
		##### UNSPECTATOR ######
		if ( $command =~ /$config->{irc_botcommandname}.* unspec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={UNSPEC}=-\x03 $nick has unspec'd $1"); &toserverpipe("unspec $1"); 
		&toirccommandlog("$nick has UNSPECTATED $1 "); return }
		##### MUTE #####
		if ( $command =~ /$config->{irc_botcommandname}.* mute.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MUTE}=-\x03 $nick muted $1"); &toserverpipe("player_var $1 mute 1");
		&toirccommandlog("$nick MUTED $1"); return }
		##### UNMUTE #####
		if ( $command =~ /$config->{irc_botcommandname}.* unmute.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={UNMUTE}=-\x03 $nick Unmuted $1"); &toserverpipe("player_var $1 mute 0"); 
		&toirccommandlog("$nick UNMUTED $1"); return }
		##### GIVEMASTER #####
		if ( $command =~ /$config->{irc_botcommandname}.* givemaster.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={GIVEMASTER}=-\x03 $nick gave master to $1"); &toserverpipe("setmaster $1 1");
		&toirccommandlog("$nick GIVEMASTER $1"); return }

		##### TAKEMASTER #####
		if ( $command =~ /$config->{irc_botcommandname}.* takemaster.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={TAKEMASTER}=-\x03 $nick took master from $1"); &toserverpipe("setmaster $1 0"); 
		&toirccommandlog("$nick TAKEMASTER $1"); return }
		##### MASTER #####
		if ( $command =~ /$config->{irc_botcommandname}.* master/i )
		{ &toserverpipe("masterwho") ;
		&toirccommandlog("$nick MASTERWHO $1"); return }

		##### MASTERMODE #####
		if ( $command =~ /$config->{irc_botcommandname}.* mastermode.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MASTERMODE}=-\x03 $nick changed mastermode to $1"); &toserverpipe("mastermode $1"); 
		&toirccommandlog("$nick MASTERMODE $1"); return }
		##### MAPCHANGE #####
		if ( $command =~ /$config->{irc_botcommandname}.*map.*(instagib|ffa|capture) (.+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MAPCHANGE}=-\x03 $nick changed  map to mode $1 map $2"); &toserverpipe("changemap $1 $2"); 
		&toirccommandlog("$nick MAP $1 $2"); return }
		##### HELP #####
		if ( $command =~ /$config->{irc_botcommandname}.*help/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={HELP}=-\x03 help can be found here http://hopmod.e-topic.info/index.php5?title=IRC_Bot"); return}
		##### WHO #####
		if ( $command =~ /$config->{irc_botcommandname}.*who/i )
		{ &toserverpipe("who"); return}
		##### DIE #####
		if ( $command =~ /$config->{irc_botcommandname}.*die/i ) 
		{&sendtoirc("\x03\x036IRC\x03         \x034-={DIE}=-\x03 $nick terminated the bot") ;
		&toserverpipe("restart_ircbot") ; 
		&toirccommandlog("$nick RESTART_IRCBOT"); return }
		##### VERSION #####
		if ( $command =~ /$config->{irc_botcommandname}.*version/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={VERSION}=-\x03 HopBot V$version by -=PunDit=- #hopmod"); return}
		##### SAUERPING
		if ( $command =~ /$config->{irc_botcommandname}.*sauerping/i )
		{ &sauerping ; return }
		##### RESTART SERVER #####
		if ( $command =~ /$config->{irc_botcommandname}.*restart.*server/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={RESTART SAUER}=-\x03 $nick restarted the server process"); &toserverpipe("restarter; shutdown");
		&toirccommandlog("$nick RESTART_SERVER"); return }
		##### SHOWALIAS #####
                if ( $command =~ /$config->{irc_botcommandname}.* showalias.*\s([0-9]+.*)/i )
                { &toserverpipe("showalias $1"); 
		&toirccommandlog("$nick SHOWALIAS $1"); return }
		##### SCORE #####
		if ( $command =~ /$config->{irc_botcommandname}.* score.*/i )
		{ &toserverpipe("score");
		&toirccommandlog("$nick SCORE"); return }
		##### SETMOTD #####
		if ( $command =~ /$config->{irc_botcommandname}.* setmotd (.*)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MOTD}=-\x03 changed to $1")
		&toserverpipe("motd = \"$1\""); 
		&toirccommandlog("$nick SETMOTD $1"); return }

		##### GETMOTD #####
                if ( $command =~ /$config->{irc_botcommandname}.* getmotd/i )
		{&sendtoirc("updating");
                &toserverpipe("getvar motd"); 
		&toirccommandlog("$nick GETMOTD"); return }

		##### GETVAR #####
		if ( $command =~ /$config->{irc_botcommandname}.* getvar (.*)/i )
                { &toserverpipe("getvar $1");
		&toirccommandlog("$nick GETVAR $1"); return }
	
		#####UPDATE#####	
		if ( $command =~ /$config->{irc_botcommandname}.* update $config->{irc_adminpw}/i )
                { &toserverpipe("updating"); &update;
                &toirccommandlog("$nick UPDATE"); return }
		
		
		
		if ( $command =~ /$config->{irc_botcommandname}/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={DOESNOTCOMPUTE}=-\x03 What the hell are you trying to tell me $nick");return }
		
		
	}
}

sub filterlog {
	my @nick;
	my $line = shift;
	
	##### VERIFICATION IP REPLACEMENT
	if ($line =~  /\(([0-9]+)[.]([0-9]+)[.]([0-9]+)[.]([0-9]+)\)/i) {
		my $ip = "$1.$2.$3.$4";
		if ( $line =~ /(.*)\([0-9]*\)/ ){ $nick[0] = $1; }
		$nick[0] =~ s/(\!|\~|\^|\*|\?|\#|\`|\||\\|\<|\>|'|"|\(|\)|\[|\]|\$|\;|\&|\\)/\\$1/g; 
		if ( $nick[0] eq "" ) { $nick[0] = "noone" }
		if ( $ip eq "" ) { $ip = "127.0.0.1" }
		
		if ( -e 'ext/bin/verify.pl') {
			my $VERIFY = `ext/bin/verify.pl --name=$nick[0] --ip=$ip --writestate=no --showmask=no`;
		$line =~ s/$ip/$VERIFY/g; } else { $line =~ s/$ip/Masked/g }
				
		#return $line;
	}
	
	
	##### TEXT FORMATTING #####
	
	##### CONNECT #####
	if ($line =~ /(\S*\([0-9]+\))(\(.+\)) connected/)
	{ return "\x039CONNECT\x03    \x0312$1\x03 $2" }
	##### DISCONNECT #####
	if ($line =~ /(\S*\([0-9]+\)) disconnected, (.+)/)
	{ return "\x032DISCONNECT\x03 \x0312$1\x03 $2" }
	##### RENAME #####
	if ($line =~ /(\S*\([0-9]+\)) renamed to (.+)/) 
	{ return  "\x032RENAME\x03     \x0312$1\x03 has renamed to \x037$2\x03"}
	##### REGISTRATION
        if ($line =~  /#register/i) 
	{return "REGISTRATION";} # Filter Server Registration
	##### CHAT #####
	if ($line =~  /(\S*\([0-9]+\)): (.+)/) 
	{ return "\x033CHAT\x03       \x0312$1\x03 --> \x033$2\x03" }# Highlight game chat green
	##### MAP CHANGE #####
	if ($line =~ /new game: (.*), (.*), (.*)/) 
	{ return "\x032NEWMAP\x03     New map \x037$3\x03 for \x037$2\x03 with \x037$1\x03 " }
	##### MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) claimed master/) 
	{ $master = $1 ; return "\x034MASTER\x03     \x0312$1\x03 took master." }
	##### RELEASE MASTER #####
	if ($line =~ /(\S*\([0-9]+\)) relinquished privileged status./) 
	{ $master = "NULL" ; return "\x034UNMASTADM\x03   \x0312$1\x03 relinquished privileged status" }
	##### KICK BAN #####
	if ($line =~ /(\S*) was kicked by (.*)/) 
	{ return "\x034KICK\x03      Master \x034$1\x03 kicked \x0312$2\x03" }
	##### KICK BAN 2
	if ($line =~ /(\S*\([0-9]+\)) kick\/banned for:(.+)\.\.\.by console./) 
	{ return "\x034KICK\x03      Console kicked \x0312$1\x03 for $2" }
	##### ADMIN #####
	if ($line =~ /(\S*\([0-9]+\)) claimed admin/) 
	{ $master = $1 ; return "\x034ADMIN\x03       \x0312$1\x03 took admin" }
	##### TEAM CHANGE
	if ($line =~ /(\S*\([0-9]+\)) changed team to (.+)/) 
	{ return "\x034CHANGETEAM \x03\x0312$1\x03 changed teams to \x037$3\x03" }
	##### MAP VOTE #####
	if ($line =~ /(\S*\([0-9]+\)) suggests (.+) on map (.+)/) 
	{ return "\x033SUGGEST\x03    \x0312$1\x03 suggests \x037$2\x03 on \x037$3\x03" }
	##### SERVER RESTART #####
	if ($line =~ /server started (.+)/) 
	{ return "\x034SERVER\x03    Server Restarted at \x037$1\x03" }
	##### MASTERMODE #####
	if ($line =~ /mastermode is now ([0-9])/) 
	{ return "\x034MASTERMODE\x03  Mastermode is now \x0312$1\x03" }
	##### WHO #####
	if ($line =~ /WHO/g) {
		while ( $line =~ /(\S*)\([0-9]*\)/g ) {
			$line =~ s/(\S*)\(([0-9]*)\)/\x0312$1\[$2\]\x03/}
	$line =~ s/WHO/\x034WHO\x03/; return $line}
	##### SCORE #####
	if ($line =~ /SCORE/g) {
		while ( $line =~ /(\S*) F([0-9]*)\/D([0-9]*)/g ) {
			$line =~ s/(\S*) F([0-9]*)\/D([0-9]*)/\x0312$1\x03 [F\x033$2\x03\/D\x034$3\x03]/}
	$line =~ s/SCORE/\x034SCORE\x03/; return $line}	
	##### GETVAR #####
        if ($line =~ /IRC .*-={GETVAR (.*)}=- is (.*)/)
        { return "\x03\x036IRC\x03         \x034-={GETVAR $1}=-\x03 is \x037$2\03" }
	##### MASTER #####
        if ($line =~ /COMMAND CURRENTMASTER (.*)/)
        { return "\x03\x036IRC\x03         \x034-={MASTER}=-\x03 is \x037$1\03" }
	##### SHOWALAIS #####
	if ($line =~ /COMMAND SHOWALIAS (.*)/) 
	{ &showalias("$1") ; return}
	##### SPECTATOR #####
	if ($line =~ /(\S*\([0-9]+\)) (.*) spectators/) 
	{ return "\x034SPECTATOR\x03 \x0312$1\x03 $2 spectators" }
	##### SERVER UPDATE #####
	if ($line =~  /(Performing server update:.*)/) 
	{ return "\x034SERVER\x03 \x0312$1\x03" }
	##### SERVER SHUTDOWN #####
	if ($line =~  /server shutdown (.*)/) 
	{ return "\x034SERVER\x03 \x0312Server Shutdown at $1\x03" }
	##### IRC BOT SHUTDOWN #####
	if ($line =~  /Terminating irc bot/) 
	{ return "\x034SERVER\x03 \x0312IRC Bot Shutdown Initiated\x03" }
	##### NEW COOP MAP #####
	if ($line =~  /(\S*\([0-9]+\)) set new map of size ([0-9]*)/) 
	{ return "\x034NEWCOOPMAP\x03 \x0312$1\x03 starts new map of size \x037$2\x03" }
	##### APPROVE MASTER #####
	if ($line =~  /(\S*\([0-9]+\)) approved for master by (.+\([0-9]+\))/) 
	{ return "\x032APPROVE\x03     \x0312$1\x03 was approved for master by \x0312$2\x03" }	
	##### GENERIC #####
	if ($line =~  /Apparently no one is connected/)
        { return "Apparently no one is connected" }
	
	return $line;
}

sub sendtoirc {
	my $send = shift; 
	$irc->yield( privmsg => $config->{irc_channel} => "\x034$send\x03" );
}

sub sauerping {
	my $sauerping = `netcat -u -v -z localhost 28785-28786 2>/tmp/.sauerping ; cat /tmp/.sauerping`;
	&sendtoirc ("$sauerping");
}

sub showalias {
	my $IP = shift;
        my @array1;
        my @array2;
        my @ITEM ;
        my %hash;
        my $hash;
	my $alias;
	my $counter = 0;
	chomp ($IP);
	if ( ! $IP ) { &sendtoirc ("Connection Number Not Found") ; return }
        open (FILE, '+<', $config->{irc_serverlogfile} ) or die $!;
        my @FILE = <FILE>;
        close (FILE);
        foreach (@FILE) {
                if ( $_ =~ /([\S]*)\Q(\E([0-9]+)\Q)\E.([0-9]+.[0-9]+.[0-9]+.[0-9]+). connected/i ) {
                        if ( "$IP" eq "$3" ) { push (@array1, $1) }
                }
        }
        foreach (@array1) { $hash{$_}++ };
        foreach (sort keys %hash) { push @array2, $_ };
	foreach (@array2) { $alias = $alias . " $_" }
	
	&sendtoirc ("\x036IRC\x03         \x034-={SHOWALIAS}=-\x03 $alias");
}

sub update {
	my $output = `svn update bin/irc.pl scripts/irc.csl`;
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
