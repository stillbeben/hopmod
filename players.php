<body bgcolor="grey">
<?php
include "config.php";

function www($command) {
        $headers = "POST /serverexec HTTP/1.0\r\nHost: 127.0.0.1\r\n".
		"Content-Length: ".strlen($command)."\r\nContent-Type: text/cubescript\r\n\r\n";
        $fp = fsockopen("127.0.0.1:7894");
        if (!$fp) return false;
        fputs($fp, $headers);
        fputs($fp, $command);
        while (!feof($fp)) {
            $ret .= fgets($fp, 1024);
        }
        fclose($fp);
		$x = null;
		$x = explode(chr(32), $ret);
		if ($x[11] == "error:")
			return false;
		else
			return $ret;
}
$sp4nk = 0;
$playercount = 0;

$x = www('log "<players>"; 
	foreach (players) [
		parameters cn
		log [@(player_name $cn) @(player_frags $cn) @(player_deaths $cn) @(player_accuracy $cn) @(player_team $cn)]
	];
	log "</players>";
	log "<server>";
	log [@mapname @timeleft @gamemode];
	log "</server>";
	log "<score>";
	log [@(teamscore "good") @(teamscore "evil")];
	log "</score>";
	');

exec("tail $path_to_log -n25 && exit", $output);

for ($i = 0; $i < count($output); $i++) 
	$log[] = $output[count($output)-1-$i];

unset($x);

echo '<div align="center"><h1>Players on the sp4nk-server</h1>';
flush();

for ($i = 0; $i < count($log); $i++) {
	if ($log[$i] == "</server>") {
		$i++;
		$m = explode(chr(32), $log[$i]);
		$map = $m[0];
		$timeleft = $m[1]; 
		$gamemode = $m[2].$m[3];
		if ($gamemode == "instactf") 
			$teammmode = true;
		continue;
	}
	
	if ($log[$i] == "</score>") {
		$i++;
		$m = explode(chr(32), $log[$i]);
		$score[0] = $m[0]; // good
		$score[1] = $m[1]; // evil
		continue;
	}

	if ($log[$i] == "<players>") {
		$x = false;
		break;
	}
	if ($log[$i] == "</players>") {
		$x = true;
		continue;
	}	

	$m = explode(chr(32), $output[$i]);
	if ($m[0] == null || $m[0] == " ")
		$log[$i]= "connecting";
	
	if ($x) {
		if (contains($log[$i], "|-sp4nk-|")) {
			$log[$i] = str_replace("|-sp4nk-|", null, $log[$i]);
			$sp4nk++;
		}
		if (contains($log[$i], "|sp4nk")) {
			$log[$i] = str_replace("|sp4nk", null, $log[$i]);
			$sp4nk++;
		}
		if (contains($log[$i], "|sp4nk|")) {
			$log[$i] = str_replace("|sp4nk|", null, $log[$i]);
			$sp4nk++;
		}
		$m = explode(chr(32), $log[$i]);	
		
		$player = "<tr><td>$m[0]</td><td>$m[1]</td><td>$m[2]</td><td>$m[3]</td></tr>";
		if ($teammmode) {
			$team[$m[4]][0] .= $player = "<tr><td>$m[0]</td><td>$m[1]</td><td>$m[2]</td><td>$m[3]</td></tr>"; 
			$team[$m[4]][1]++;
		}
		else 
			$players .= "<tr><td>$m[0]</td><td>$m[1]</td><td>$m[2]</td><td>$m[3]</td></tr>";
		$playercount++;
	}
}
echo "<h2>Mode: $gamemode Map: $map Players: $playercount Timeleft: $timeleft Minute(s)</h2>";
echo '<table border="1">';
if (!$teammmode) {
	echo '<tr><td>Name</td><td>Frags</td><td>Deaths</td><td>Acc</td></tr>';
	echo $players;
}
else {
	if ($score[0] > $score[1]) {
		echo "<tr><td><b>Team: good</td><td><b>Scored-Flags: $score[0]</b></td><td colspan=\"2\"><b>Players: ".$team["good"][1]."</b></td></tr>";
		echo '<tr><td>Name</td><td>Frags</td><td>Deaths</td><td>Acc</td></tr>';
		echo $team["good"][0];
		
		echo "<tr><td><b>Team: evil</td><td><b>Scored-Flags: $score[1]</b></td><td colspan=\"2\"><b>Players: ".$team["evil"][1]."</b></td></tr>";
		echo '<tr><td>Name</td><td>Frags</td><td>Deaths</td><td>Acc</td></tr>';	
		echo $team["evil"][0];
	}
	else {	
		echo "<tr><td><b>Team: evil</td><td><b>Scored-Flags: $score[1]</b><td colspan=\"2\"><b>Players: ".$team["evil"][1]."</b></td></tr>";
		echo '<tr><td>Name</td><td>Frags</td><td>Deaths</td><td>Acc</td></tr>';
		echo $team["evil"][0];
		
		echo "<tr><td><b>Team: good</td><td><b>Scored-Flags: $score[0]</b></td><td colspan=\"2\"><b>Players: ".$team["good"][1]."</b></td></tr>";
		echo '<tr><td>Name</td><td>Frags</td><td>Deaths</td><td>Acc</td></tr>';
		echo $team["good"][0];
	}
	
}
echo '</table></div>';

function contains($var, $text) {
	if (strlen(str_replace($text, null, $var)) < strlen($var)) 
		return true;
	else 	
		return false;
}

?>
</body>

