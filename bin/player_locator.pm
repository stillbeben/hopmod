#!/usr/bin/perl

use IO::Socket;
use threads;
use threads::shared;
use LWP::Simple;

my @data :shared;
my @playerstats :shared;

my $thread = async {&loop};

sub loop {
        my $servers = get('http://sauerbraten.org/masterserver/retrieve.do?item=list');
        my @serverlist = split(/addserver /, $servers);
        while (1==1) {
                foreach (@serverlist) {
                        if ( $_ =~ /([0-9]*\.[0-9]*\.[0-9]*\.[0-9]*)./ ) {
                                &Process($1) ;
                        }
                }
                @playerstats = @data;
                undef @data;
                sleep 60;
        }
}






sub Process {
                my $datagram;
                my $server = shift;
                my $socket = new IO::Socket::INET (PeerAddr => $server,
                        PeerPort => '28786',
                        Proto    => "udp",
                        Type     => SOCK_DGRAM) or warn "Socket Connect on $server failed $!\n";
                $socket->send(pack('ccc', 0, 1, -1),0,4) or warn $!;
                my $rin = '';
                my $rout;
                vec($rin, fileno($socket), 1) = 1;
                if (!select($rout = $rin, undef, undef, .5)) {return}
                $socket->recv($datagram,50);
                my @playerdata = unpack('cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc', $datagram);
                if ( ! defined $playerdata[4] ) { $playerdata[4] = 0 }
                if ( $playerdata[4] >= 103 ) {
                        my $length = @playerdata;
                        if ($length > 7 ) {
                                if ($playerdata[6] eq "-10") {
                                        for ($a = 8; $a <= $length; $a++) {
                                                my $rin = '';
                                                vec($rin, fileno($socket), 1) = 1;
                                                if (!select($rin, undef, undef, .250)) {return}
                                                $socket->recv($datagram,50);
                                                @playerdata = unpack('c7xZ*Z*ccccccccccccccccccccccccccccc', $datagram);
                                                push (@data, "$server $playerdata[7] $playerdata[8] $playerdata[9] $playerdata[10] $playerdata[11] $playerdata[12] $playerdata[13] $playerdata[14] $playerdata[15] $playerdata[16] $playerdata[17]");
                                        }
                                }
                        }
                return 0;
        	}
}
sub lookup {
        my $search = shift;
        my @lookup = ("81.169.135.134 TC2" , "77.37.11.78 OBI", "85.214.113.69 TC1", "85.214.20.174 HOF", "24.192.20.218 QS2", "71.238.139.48 QS3", "24.192.115.69 QS1", "91.121.10.132 COE", "85.214.94.222 PSL");
        foreach (@lookup) {
                my @check = split(/ /, $_);
                if ($search =~ /[0-9]*\.[0-9]*\.[0-9]*\.[0-9]*/) {
                        if ($check[0] eq $search) { return $check[1] }
                } else {
                        if ($check[1] eq $search) { return $check[0] }
                }
        }
        return $search;
}
sub find {
        my $toirc = shift;
        my $search = shift;
        my $channel = shift;
        my $cn = shift;
        my @stats;
        my $stats;
        my $lookup;
        undef @split;
        $search = &lookup($search); ## Name Table Lookup

        if (! $search) { $search = "|RB| {QS} }TC{ w00p woop sp4nk| ws| novi hf"};
        my @searchitems = split (/ /, $search);
        foreach (@searchitems) {
                my $item = $_;
                foreach (@playerstats) {
                        if ($toirc eq "1") {
                                if ( $_ =~ /\Q$item\E/i ) { @stats = split(/ /, $_); my $lookup = &lookup($stats[0]);push(@split, "\x0312$stats[1]\x03[\x037 $lookup \x03]") }
                        } else {
                                if ( $_ =~ /\Q$item\E/i ) { @stats = split(/ /, $_); my $lookup = &lookup($stats[0]);push(@split, "$stats[1]\[$lookup]") }
                        }
                }
        }
        if ($toirc eq "1") {
                if (@split) { &splittoirc($channel, "\x03\x036IRC\x03         \x034-={FIND}=-\x03 is") }
        } else {
                &toserverpipe("privmsg [$cn] [Found: @split]");
        }
        return;

}

return 1;
