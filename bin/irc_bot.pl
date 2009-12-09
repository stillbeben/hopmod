#!/usr/bin/perl 

use LWP::UserAgent;
use HTTP::Request;
use warnings;
use strict;
use POE qw(Component::IRC::State Component::IRC::Plugin::AutoJoin Component::IRC::Plugin::Connector Component::IRC::Plugin::FollowTail);
use Config::Auto;
use vars qw($dynamic_reload $mute_status $cn $config $irc @playerstats @serverlist @data $rrd $topriv $version $hopvar $rev @split);

$version = ".01NG";
print "INFO	: Starting HopBot NG V$version support at #hopmod\@irc.gamesurge.net \n";
print "INFO	: Bot CORE is			LOADED\n";

if ( eval { require Module::Reload::Selective } ) {
        print "INFO	: Dynamic Reloading is		LOADED\n";
	$dynamic_reload = 1;
} else {
        print "ERROR	: Dynamic Reloading module is		DISABLED\n\n";
        print "ERROR MSG: $@\n";
	$dynamic_reload = 0;	
}

if ( eval { require 'bin/Command.pm' } ) {
	print "INFO	: Command module is		LOADED\n";
} else {
	print "ERROR	: Command module is		DISABLED\n\n";
	print "ERROR MSG: $@\n";
	exit
}

if ( eval { require 'bin/Filter.pm' } ) {
        print "INFO	: Filter module is		LOADED\n";
} else {
        print "ERROR    : Filter module is		DISABLED\n\n";
        print "MSG:	$@\n";
	exit
}

$config = Config::Auto::parse("../conf/ircbot.conf" , format => "equal");
if ( $config->{irc_player_locator} eq "1" ) {
        if ( eval { require 'bin/player_locator.pm' } ) {
                print "INFO	: Player Locator module is	LOADED\n";
        } else {
                print "ERROR	: Player Locator module is		DISABLED\n\n";
                print "MSG	: $@\n";
        }
}

$mute_status = "0";
$Module::Reload::Selective::Options->{ReloadOnlyIfEnvVarsSet} = 0;
$Module::Reload::Selective::Options->{ReloadOnlyIfEnvVarsSet} = 0;
&toserverpipe("set irc_pid $$"); #Send the server my pid for restarting purposes.

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
	if ( $msg =~ /^$config->{irc_botcommandname}/i  )                            {
		my $poco_object = $sender->get_heap();
                if ( ! $poco_object->is_channel_operator( $config->{irc_channel}, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
		if ( my @output=Command::Process($sender, $nick, $msg, $channel) ) {
			&sendtoirc($config->{irc_channel}, "\x03\x036IRC\x03         \x034-/$output[0]/-\x03 $output[1]");
		}
	}
}
sub on_private {
	my ( $kernel, $sender, $who, $where, $msg ) = @_[ KERNEL, SENDER, ARG0, ARG1, ARG2 ];
	my $nick = ( split /!/, $who )[0];
	my $channel = $where->[0];
	my $ts = scalar localtime;
	print "INFO	: [$ts] <$nick:$channel> $msg\n";
        if ( $config->{irc_channel} eq "1" ) { print "[$ts] <$nick:$channel> $msg\n"; }
        if ( $msg =~ /^$config->{irc_botcommandname}/i  )                            {
                my $poco_object = $sender->get_heap();
                if ( ! $poco_object->is_channel_operator( $config->{irc_channel}, $nick )) { &sendtoirc("Sorry you must be an operator to issues commands"); return }
                if ( my @output=Command::Process($sender, $nick, $msg, $nick)  ) {
                        &sendtoirc($nick, "\x03\x036IRC\x03         \x034-/$output[0]/-\x03 $output[1]");
                }
        }
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
	&sendtoirc($config->{irc_channel}, Filter::Message($input));
	&toserverpipe("set irc_pid  $$"); # Regular refresh for safetys sake
}
sub toprivateirc {
	my $send = shift; 
	$irc->yield( privmsg => $topriv => "$send" );
}
sub sendtoirc {
	my $channel = shift;
	my $send = shift; 
	if ($mute_status == "0") {
		$irc->yield( privmsg => $channel => "$send" );
	} 
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
        $post->content_type("text/x-cubescript");
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
