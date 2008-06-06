#!/usr/bin/perl
use warnings;
use strict;
use POE qw(Component::IRC::State Component::IRC::Plugin::AutoJoin Component::IRC::Plugin::Connector Component::IRC::Plugin::FollowTail);
use Switch;
use Config::Auto;
use vars qw($master $irccommand_log $repeatcount $config $version $lastline $tailfile $ircport $username $botcommandname $zippy @zippy $network $botname $channel $pwd $server_pipe $state_tbl $server_log @word @STATE $scn $sip $sname);

$pwd = `cat /tmp/.pwd`; chomp ($pwd);
$server_pipe    = $pwd . '/serverexec';
$state_tbl      = $pwd . '/ext/tmp/state.tbl';
$server_log     = $pwd . '/logs/server.log';
$irccommand_log	= $pwd . '/logs/irccommand.log';
$repeatcount = 1 ;
$version = "1.10"; #<----Do NOT change this or I will kill you
$master = "NULL";




$config = Config::Auto::parse("../conf/irc.conf" , format => "space");
$botcommandname = $config->{botcommandname};

#Config File Overrides
if ( defined $config->{logfile} ) { $server_log = "$config->{logfile}" }
if ( defined $config->{irccommandlog} ) { $irccommand_log = "$config->{irccommandlog}" }
if ( defined $config->{serverpipe} ) { $server_pipe = "$config->{serverpipe}" }



print "Starting SauerBot V$version by -={Pundit}=- #quicksilver\@irc.gamesurge.net \n";
if ( -e '$pwd/ext/bin/verify.pl') { 
	print "[Detected AuthMod, enabling verification features]\n"; 
	print "Checking for command pipe availability";
	if ( -e '$pwd/serverexec' ) { print "[PASS]\n" } else { print "[FAIL]\n some commands may not function properly\n" }
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
	},
	package_states => [ 
	main => [ qw(bot_start lag_o_meter ) ],
	],
	);

sub bot_start {
	my $kernel  = $_[KERNEL];
	my $heap    = $_[HEAP];
	my $session = $_[SESSION];
	my %channels = ( $config->{channel}   => '' ); 
	$heap->{connector} = POE::Component::IRC::Plugin::Connector->new();
	$irc->plugin_add( 'FollowTail' => POE::Component::IRC::Plugin::FollowTail->new( filename => $server_log,));
	$irc->plugin_add('AutoJoin', POE::Component::IRC::Plugin::AutoJoin->new( Channels => \%channels, RejoinOnKick => 1));
	$irc->plugin_add( 'Connector' => $heap->{connector} );
	$irc->yield( register => "all" );
	my $nick = $config->{botname};
	$irc->yield( connect =>
		{ Nick => $nick,
			Username => $config->{username},
			Ircname  => $config->{username},
			Server   => $config->{network},
			Port     => $config->{ircport},
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
	$irc->yield( join => $config->{channel} );
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

sub on_tail_input {
	my ($kernel, $sender, $filename, $input) = @_[KERNEL, SENDER, ARG0, ARG1];
        $irc->yield( privmsg => $config->{channel} => &filterlog($_[ARG1]) );
}
sub process_command {
	my $sender = shift;
	my $who = shift;
	my $command = shift;
	my $channel = $config->{channel};
	my $nick = ( split /!/, $who )[0];
    	my $poco_object = $sender->get_heap();
	if ( $command =~ /$botcommandname/i  )                            {
		if ( ! $poco_object->is_channel_operator( $channel, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
		##### COMMAND PROCESSING #####
		
		##### SAY
		if ( $command =~ /$botcommandname.* say\s(.+)/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={SAY}=-\x03 Console($nick): \x034$1\x03"); &toserverpipe("console \"Console($nick): $1\"");
                &toirccommandlog("Console($nick): $1");  return }	
		##### KICK
		if ( $command =~ /$botcommandname.* kick.* ([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={KICK}=-\x03 $nick kicked $1"); &toserverpipe("kick $1"); 
		&toirccommandlog("No problem $nick, consider $1 gone"); return }
		##### CLEARBANS
		if ( $command =~ /$botcommandname.* clearbans/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034CLEARBANS\x03 $nick has cleared bans"); &toserverpipe("clearbans"); 
		&toirccommandlog("Master $nick I have cleared bans for you"); return }
		##### SPECTATOR
		if ( $command =~ /$botcommandname.* spec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={SPEC}=-\x03 $nick has spec'd $1"); &toserverpipe("spectator $1 1"); 
		&toirccommandlog("$nick I have spec'd $1 "); return }
		##### TICKLE
		if ( $command =~ /tickle.* $botcommandname.*/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={WISDOM}=-\x03 $zippy[ rand scalar @zippy ]"); return }
		##### HI
		if ( $command =~ /hi.* $botcommandname.*/i )
		{ &sendtoirc("hey $nick hows it going?");return }
		##### UNSPECTATOR
		if ( $command =~ /$botcommandname.* unspec.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03]         \x034-={UNSPEC}=-\x03 $nick has unspec'd $1"); &toserverpipe("spectator $1 0"); 
		&toirccommandlog("$nick I have unspec'd $1 "); return }
		##### MUTE
		if ( $command =~ /$botcommandname.* mute.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MUTE}=-\x03 $nick muted $1"); &toserverpipe("user_var $1 mute 1");
		&toirccommandlog("$nick Muting $1, what a spammer"); return }
		##### UNMUTE
		if ( $command =~ /$botcommandname.* unmute.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={UNMUTE}=-\x03 $nick Unmuted $1"); &toserverpipe("user_var $1 mute 0"); 
		&toirccommandlog("$nick Unmuted $1, I guess he learned his lesson"); return }
		##### GIVEMASTER
		if ( $command =~ /$botcommandname.* givemaster.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={GIVEMASTER}=-\x03 $nick gave master to $1"); &toserverpipe("master $1 1");
		&toirccommandlog("ok ok $nick I gave master to $1"); return }
		##### TAKEMASTER
		if ( $command =~ /$botcommandname.* takemaster.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={TAKEMASTER}=-\x03 $nick took master from $1"); &toserverpipe("master $1 0"); 
		&toirccommandlog("ok ok $nick I stole master from $1"); return }
		##### MASTER
		if ( $command =~ /$botcommandname.* master .*/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MASTERCHECK}=-\x03 current master [\x0312$master\x03]") ; return }
		##### MASTERMODE
		if ( $command =~ /$botcommandname.* mastermode.*\s([0-9]+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MASTERMODE}=-\x03 $nick changed mastermode to $1"); &toserverpipe("mastermode $1"); 
		&toirccommandlog("Gotcha $nick changing mastermode to $1"); return }
		##### MAPCHANGE
		if ( $command =~ /$botcommandname.*map.*(instagib|ffa|capture) (.+)/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={MAPCHANGE}=-\x03 $nick changed  map to mode $1 map $2"); &toserverpipe("changemap $1 $2"); 
		&toirccommandlog("Good choice $nick changing the map to mode $1 map $2"); return }
		##### HELP
		if ( $command =~ /$botcommandname.*help/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={HELP}=-\x03 help can be found here http://hopmod.e-topic.info/index.php5?title=IRC_Bot"); return}
		##### WHO
		if ( $command =~ /$botcommandname.*who/i )
		{ &show_state; return}
		##### CLEARSTATE
		if ( $command =~ /$botcommandname.*clearstate/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={STATE}=-\x03 $nick cleared state"); undef @STATE ; return}
		##### DIE
		if ( $command =~ /$botcommandname.*die/i ) 
		{&sendtoirc("\x03\x036IRC\x03         \x034-={DIE}=-\x03 $nick terminated the bot") ;&toirccommandlog("$nick has killed the bot");
		exit }
		##### VERSION
		if ( $command =~ /$botcommandname.*version/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={VERSION}=-\x03 SauerBot V$version by -=PunDit=- #quicksilver"); return}
		##### SAUERPING
		if ( $command =~ /$botcommandname.*sauerping/i )
		{ &sauerping ; return }
		##### RESTART SERVER
		if ( $command =~ /$botcommandname.*restart.*server/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={RESTART SAUER}=-\x03 $nick restarted the server process"); `cd $pwd; ./sauerserverctl restart`;
		&toirccommandlog("Restarting Server"); return }
		##### SHOWALIAS
                if ( $command =~ /$botcommandname.* showalias.*\s([0-9]+.*)/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={ALIAS}=-\x03 $nick checking for aliases on CN $1"); &showalias($1); 
		&toirccommandlog("Gotcha $nick checking for aliases"); return }
		##### SCORE
		if ( $command =~ /$botcommandname.* score.*/i )
                { &sendtoirc("\x03\x036IRC\x03         \x034-={SCORE}=-\x03 $nick checking the score"); 
		&toserverpipe("
		output = []
scores = \"\"

loop i (listlen (clients)) [
   cn = (at (clients) \$i)
   line = (format \"\%4 F\%1/D\%2/A\%3 \%5\" (get_player_frags \$cn) (get_player_deaths \$cn) (get_player_accuracy \$cn) (get_player_name \$cn))
   scores = (concat \$scores \$line)
]
if (! (strcmp \$scores \"\" )) [
server_log (concat \"SCORE      \" \$scores)	
]			
if (strcmp \$scores \"\" ) [
server_log \"Apparently no one is connected\"
]	
			");
		 return }
		
		if ( $command =~ /$botcommandname/i )
		{ &sendtoirc("\x03\x036IRC\x03         \x034-={DOESNOTCOMPUTE}=-\x03 What the hell are you trying to tell me $nick");return }
		
		
	}
}

sub filterlog {
	my @nick;
	my $line = shift;
	
	##### SPAM PROTECTION
	if ($line eq $lastline) { 
		
		if ( $repeatcount < $config->{spamlines} ) {
			$repeatcount++; 
		} else {
			if ( $repeatcount == $config->{spamlines} ) { $repeatcount++ ; return "\x034SPAM Detected! Discarding subsequent repeats -->\x0312 " . $line } else { return }
		}
	} else { $repeatcount = 1 }	
	
	$lastline = $line;
	
	##### VERIFICATION IP REPLACEMENT
	if ($line =~  /([0-9]+)[.]([0-9]+)[.]([0-9]+)[.]([0-9]+)/i) {
		my $ip = "$1.$2.$3.$4";
		if ( $line =~ /(.*)\([0-9]*\)/ ){ $nick[0] = $1; }
		$nick[0] =~ s/(\!|\~|\^|\*|\?|\#|\`|\||\\|\<|\>|'|"|\(|\)|\[|\]|\$|\;|\&|\\)/\\$1/g; 
		if ( $nick[0] eq "" ) { $nick[0] = "noone" }
		if ( $ip eq "" ) { $ip = "127.0.0.1" }
		if ( -e '$pwd/ext/bin/verify.pl') {
			my $VERIFY = `$pwd/ext/bin/verify.pl --name=$nick[0] --ip=$ip --writestate=no --showmask=no`;
		$line =~ s/$ip/$VERIFY/g; } else { $line =~ s/$ip/Masked/g }
		
		if ($line =~ /(.+)\(([0-9]+)\)\((.+)\).connected./ )
		{ $sname = $1 ; $scn = $2 ; $sip = $ip ; &add_state}
		
		if ($line =~ /(.+)\(([0-9]+)\)\((.+)\).disconnected./ )
		{ $sname = $1 ; $scn = $2 ; $sip = $ip ; &remove_state}
		
		#return $line;
	}
	
	
	##### TEXT FORMATTING #####
	
	##### CONNECT
	if ($line =~ /(.+\([0-9]+\))(\(.+\)) connected./)
	{ return "\x039CONNECT\x03    \x0312$1\x03 $2" }
	##### DISCONNECT
	if ($line =~ /(.+\([0-9]+\))(\(.+\)) disconnected./)
	{ return "\x032DISCONNECT\x03 \x0312$1\x03 $2" }
	##### RENAME
	if ($line =~ /(.+)\(([0-9]+)\) has renamed to (.+)\(.+\)/) 
	{ $sname = $3 ; $scn = $2 ; &update_state ; return  "\x032RENAME\x03     \x0312$1$2\x03 has renamed to \x037$3$4\x03"}
	##### REGISTRATION
        if ($line =~  /#register/i) 
	{return "REGISTRATION";} # Filter Server Registration
	##### CHAT
	if ($line =~  /(.+\([0-9]+\)) says (.+)/) 
	{ return "\x033CHAT\x03       \x0312$1\x03 screams \x033$2\x03" }# Highlight game chat green
	##### MAP CHANGE
	if ($line =~ /New game (.+) (.+) @ (.+)/) 
	{ return "\x032NEWMAP\x03     New map \x037$1\x03 on \x037$2\x03 at $3" }
	##### MASTER
	if ($line =~ /(.+\([0-9]+\)) claimed master./) 
	{ $master = $1 ; return "\x034MASTER\x03     \x0312$1\x03 took master." }
	##### RELEASE MASTER
	if ($line =~ /(.+\([0-9]+\)) relinquished master\/admin./) 
	{ $master = "NULL" ; return "\x034UNMASTADM\x03   \x0312$1\x03" }
	##### KICK BAN
	if ($line =~ /(.+\([0-9]+\)) kick\/banned for:(.+)\.\.\.by master./) 
	{ return "\x034KICK\x03      Master $master kicked \x0312$1\x03 for $2" }
	##### KICK BAN 2
	if ($line =~ /(.+\([0-9]+\)) kick\/banned for:(.+)\.\.\.by console./) 
	{ return "\x034KICK\x03      Console kicked \x0312$1\x03 for $2" }
	##### ADMIN
	if ($line =~ /(.+\([0-9]+\)) claimed admin./) 
	{ $master = $1 ; return "\x034ADMIN\x03       \x0312$1\x03 took admin" }
	##### TEAM CHANGE
	if ($line =~ /(.+\([0-9]+\)) changed teams from (.+) to (.+)./) 
	{ return "\x034CHANGETEAM\x03\x0312$1\x03 changed teams from \x037$2\x03 to \x037$3\x03" }
	##### MAP VOTE
	if ($line =~ /(.+\([0-9]+\)) suggests (.+) on map (.+)/) 
	{ return "\x033SUGGEST\x03    \x0312$1\x03 suggests \x037$2\x03 on \x037$3\x03" }
	##### SERVER RESTART
	if ($line =~ /Server started (.+)/) 
	{ return "\x034SERVER\x03    Server Restarted at \x03$1\x03" }
	##### MASTERMODE
	if ($line =~ /mastermode is now ([0-9])/) 
	{ return "\x034MASTERMODE\x03  Mastermode is now \x0312$1\x03" }
	##### SAY
	if ($line =~ /	^WHO/g) {
		while ( $line =~ /(\S*)\([0-9]*\)/g ) {
			$line =~ s/(\S*)\(([0-9]*)\)/\x0312$1\[$2\]\x03/ ;	
		}
	return $line
	}
	

	return $line;
}

sub show_state {
&toserverpipe("



                output = []
scores = \"\"

loop i (listlen (clients)) [
   cn = (at (clients) \$i)
   line = (format \"\%1(\%2)\" (get_client_name \$cn) (\$cn)) 
   scores = (concat \$scores \$line)
]
if (! (strcmp \$scores \"\" )) [
log (concat \"WHO      \" \$scores)
]
if (strcmp \$scores \"\" ) [
P
log \"Apparently no one is connected\"
]
");
	
}
sub sendtoirc {
	my $send = shift; 
	$irc->yield( privmsg => $config->{channel} => "\x034$send\x03" );
}

sub sauerping {
	my $sauerping = `netcat -u -v -z localhost 28785-28786 2>/tmp/.sauerping ; cat /tmp/.sauerping`;
	&sendtoirc ("$sauerping");
}

sub showalias {
        my $CN = shift;
        my @array1;
        my @array2;
        my $IP = "";
        my @ITEM ;
        my %hash;
        my $hash;
	my $alias;
	if ( $CN =~  /(^[0-9]*[0-9]+$)/i) {
		open (FILE, '+<', $state_tbl)  ; my @FILE = <FILE>; close (FILE);
		if ( ! -e $state_tbl ) { @FILE = @STATE }
		foreach (@FILE) {
			split (/ /, $_); if ( $_[2] eq $CN ) { $IP = $_[1] }
		}
	} else { $IP = $CN }
	if ( ! $IP ) { &sendtoirc ("Connection Number Not Found") ; return }
        open (FILE, '+<', $server_log) or die $!;
        my @FILE = <FILE>;
        close (FILE);
        foreach (@FILE) {
                if ( $_ =~ /([\S]*)\Q(\E([0-9]+)\Q)\E.([0-9]+.[0-9]+.[0-9]+.[0-9]+). connected./i ) {
                        if ( "$IP" eq "$3" ) { push (@array1, $1) }
                }
        }
        foreach (@array1) { $hash{$_}++ };
        foreach (sort keys %hash) { push @array2, $_ };
	foreach (@array2) {  $alias = $alias . " $_" }
	&sendtoirc ($alias);
}

sub toserverpipe {
        my $command = shift;
        open (FILE, '>', $server_pipe ) or die $!; print FILE "$command\n"; close (FILE);
}
sub toirccommandlog {
        my $send = shift;
        my $ts = scalar localtime;
        open (FILE, '>>', $irccommand_log ) or warn $!;
        print FILE ("[$ts] $send\n");
        close (FILE);
}
sub remove_state {
	my @DATA;
	if ( $master eq "$sname($scn)" ) { $master = "NULL" }
        foreach (@STATE) {
                split (/\s/,$_);
                if ( $_[1] eq $sip ) { } else { push (@DATA, $_) }
        }
	@STATE = @DATA;
	return;
}
sub update_state {
	my @DATA;
        foreach (@STATE) {
                split (/\s/, $_);
                if ( $_[2] eq $scn && $_[0] ne $sname ) { push (@DATA, "$sname $sip $scn") } else { push (@DATA, $_) } 
        }
	@STATE = @DATA;
	return;
}
sub add_state {
	my @DATA;
        foreach (@STATE) {
                split (/\s/, $_);
                if ( $_[0] eq $sname && $_[1] eq $sip ) {return }
        }
	push (@STATE, "$sname $sip $scn") ;
	return;
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
