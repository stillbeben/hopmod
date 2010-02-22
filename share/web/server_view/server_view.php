<?php
// How to set this up:
// For example, your server is running on port 28785(normal server port), then you need to call:
// x_info('94.23.16.174', 28786); flush(); // yes, 28786 (=info port)
// ATTENTION: IF YOU DONT GET A RESULT, TRY TO CHANGE THE STREAM_TIMEOUT (50000 = 50ms)
// Running Example: http://sp4nk.net/servers
echo 
'<style type="text/css">
table { 
	font-size:100%;
	width:50%;
	font-family:Tahoma;
}
</style>';
echo '<table border="1" align="center">';
echo '<tr><td>Server</td><td>Mode</td><td>Map</td><td>Time</td><td>Players</td></tr>';
x_info('94.23.16.174', 40001); flush();
x_info('94.23.16.174', 28786); flush();
x_info('94.23.16.174', 14001); flush();
x_info('94.23.16.174', 15001); flush();
x_info('94.23.16.174', 44445); flush();
echo '</table>';


function x_info($ip, $port) {
	$x = server_info_name_map($ip, $port);
	if (empty($x['slots'])) return; // return if server is down
	echo '<tr><td>'.$x['server'].'</td><td>'.$x['mode'].'</td><td>'.$x['map'].'</td><td>'.$x['time'].'</td><td>'.$x['players'].'/'.$x['slots'].'</td></tr>'."\n";
}

function server_info_name_map($ip, $port) {
	$s = stream_socket_client("udp://".$ip.":".$port);
	stream_set_timeout($s, 0, 50000);
	fwrite($s, chr(0x19).chr(0x01)); 
	$b = new buf();
	$g = fread($s, 4096);
	$b->stack = unpack("C*", $g);
	 
	/*for ($i = 0; $i < 100; $i++) {
		echo $i.'=>'.$b->getint()."<br>";
		flush();
	}*/
	 
	$b->getint();
	$b->getint();
	$se['players'] = $b->getint();
	$b->getint();
	$b->getint();
	$se['mode'] = getmode($b->getint());
	$se['time'] = $b->getint();
	$se['slots'] = $b->getint();
	$b->getint();
	$se['map'] = $b->getstring();	
	$se['server'] = $b->getstring();
	if (empty($se['server'])) $se['server'] = 'No-Desc';
	return $se;
}

function getmode($int) {
	switch($int) {
		case 0: return 'ffa/default';
		case 1: return 'coop edit';
		case 3: return 'instagib';
		case 5: return 'efficiency';
		case 9: return 'capture';
		case 10: return 'regen capture';
		case 11: return 'ctf';
		case 12: return 'insta ctf';
		case 13: return 'protect';
		case 14: return 'insta protect';
		default: return 'unknown';
	}
}

class buf {
	public $stack = array();
	function getc() { 
		return array_shift($this->stack);
	}
	function getint() {  
		$c = $this->getc();
		if ($c == 0x80) { 
			$n = $this->getc(); 
			$n |= $this->getc() << 8; 
			return $n;
		}
		else if ($c == 0x81) {
			$n = $this->getc();
			$n |= $this->getc() << 8;
			$n |= $this->getc() <<16;
			$n |= $this->getc()<<24;
			return $n;
		}
		return $c;
	}
	function getstring($len=10000) {
		$r = ""; $i = 0; 
		while (true) { 
			$c = $this->getint();
			if ($c == 0) { 
				return $r;
			} 
			$r .= chr($c);
		} 
	}
}
?>