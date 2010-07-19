<?php
session_start();

if (!empty($_POST['input']))
{
        $st = $_POST['input'];
        $command = split(" ", $st);
        switch($command[0])
        {
                case 'login':
			$command = "check_admin_password " . $command[1];
			$response = getHop($command);
                        if ($response == 1)  { $_SESSION['logged_in'] = true; print "Success - Logged in\n"; serverStatus(); players();} else { print "Error - Login Failed $response"; }
                        break;
                case 'error':
                        header('HTTP/1.0 Internal Server Error');
                        print 'AAAAAAAAAHHHHHHHHHH!!! That hurt!!!';
                        break;
		case 'status':
			checkAuth();
			serverStatus();
			players();
			break;

                case 'help':
                        print <<<HELP

Welcome to hcon the Hopmod AJAX Terminal.
        	help [ Print this help message ]
	status [ Shows a quick status of the server ]
        	login <serverpass> [ Login to the server ]
	
Any command not listed will be sent directly to the server. You can find the command
reference at http://hopmod.e-topic.info/index.php5?title=Server_API_Reference

HELP;
                        break;
                default:
			checkAuth();
                        print getHop($st);
        }
} else { 
	if ( $_SESSION['first_load'] == false ){ 
		$hostname = getHop('get servername');
		print "Connected to [ $hostname ] Type 'help' for help." ;
		$_SESSION['first_load'] = true;
	} else { 
		print 'hopmod>'; 
	}
}

function getHop($cubescript) {
        $content_length = strlen($cubescript);
        $headers= "POST /serverexec HTTP/1.0\r\nContent-type: text/x-cubescript\r\nHost: 127.0.0.1:7894\r\nContent-length: $content_length\r\n\r\n";
        $fp = fsockopen("127.0.0.1:7894");
        if (!$fp) return "Not Connected";
        fputs($fp, $headers);
        fputs($fp, $cubescript);
        $ret = "";
        while (!feof($fp)) {
                $ret = fgets($fp, 1024);
        }
        fclose($fp);
	if (preg_match("/type has no supported representation/", $ret) ) { return; }
        return $ret;
}

function serverStatus () {
	$map = getHop("get map");
        $gamemode = getHop("get gamemode");
        $timeleft = getHop("get timeleft");
        $playercount = getHop("get playercount");
        $speccount = getHop("get speccount");
        $botcount = getHop("get botcount");
        print "
----------[ Server Status ]----------
Map: $map\n Mode: $gamemode\n Time: $timeleft\n Players: $playercount Specs: $speccount Bots: $botcount\n";
	return;
}

function checkAuth() {
	if ( $_SESSION['logged_in'] === false ) {
		print "You must login first. Type help for more information";
		exit;
        } else { return; }
}

function players() {
	$players = split(" ", getHop("players"));
	if ( ! preg_match("/[0-9]/",$players[0]) )  return; 
	foreach ( $players as $player ) {
		$player_list .= getHop("player_name $player") . "($player) ";
	}
	print "\n----------[ Player List ]----------\n";
	print $player_list;
}
?>

