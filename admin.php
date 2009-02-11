<?php
include "config.php";
$sname = "sp4nk-server";
$maxlines = 26; //maxplayers+10 (16+10)

if ($admin_pw_control) {
	if (!isset($_POST["whatever"])) {
		if (isset($_COOKIE["sauerserver"])) 
			$pass = $_COOKIE["sauerserver"];
		else 
			login();
	}
	else
		$pass = $_POST["whatever"];
}
	
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
			return true;
}

function html($rtime = 999, $sname = null) {
	echo '
		<html>
		<head>
			<style type="text/css"><!-- 
				a:link{
					color:#008000; 
					text-decoration: none;
				}
				a:visited{
					color:#008000;
					text-decoration: none; 
				}
				a:hover{
					color:#f00;
					text-decoration: underline; 
				}
				a:active{
					color:#f00;
					text-decoration: none; 
				}
				input {
				  color:#050;
				  font: bold 84% \'trebuchet ms\',helvetica,sans-serif;
				  background-color:#fed;
				  border:1px solid;
				  border-color: #696 #363 #363 #696;
				}
			--></style>
			<script type="text/javascript">
			<!--
				function newwindow(w,h,webaddress) {
					var viewimageWin = 
window.open(webaddress,\'New_Window\',\'toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=Yes,resizable=no,copyhistory=no,width=\'+w+\',height=\'+h);
					viewimageWin.moveTo(screen.availWidth/2-(w/2),screen.availHeight/2-(h/2));
				}
			//-->
			</script>
			<meta http-equiv="refresh" content="'.$rtime.'; URL=admin.php">
		</head>
		<body bgcolor="grey">
	    <div align="center"><h1>Adminpanel for '.$sname.'</h1>';
}

function login() {
	html();
	echo '<form action="admin.php" method="post">
		  Password: <input type="password" name="whatever">
		  <input type="submit" name="login" value="login">';
	exit;
}

$playercount = 0;

$x = www('log "<players>"; 
	foreach (players) [
		parameters cn
		log [@(player_name $cn) @(player_frags $cn) @(player_deaths $cn) @(player_accuracy $cn) @(player_team $cn) @cn @(player_status $cn) @(player_priv $cn) @(player_ping $cn) @(player_ip 
$cn) @(country (player_ip $cn))]
	];
	log "</players>";
	log "<server>";
	log [@mapname @timeleft @gamemode];
	log "</server>";
	log "<score>";
	try [log [@(teamscore "good") @(teamscore "evil")]] []
	log "</score>";
	log "<admin>";
	if (adminpass "'.$pass.'") [log "admin:true"] [log "admin:false"]
	log "</admin>";
	');

exec("tail $path_to_log -n$maxlines && exit", $output);

for ($i = 0; $i < count($output); $i++) 
	$log[] = $output[count($output)-1-$i];

unset($x);

for ($i = 0; $i < count($log); $i++) {
	if ($log[$i] == "</admin>") {
		if ($admin_pw_control) {
			$i++;
			if ($log[$i] == "admin:true") {
				$logged_in = true;
				if (!isset($_COOKIE["sauerserver"]))
					setcookie("sauerserver", $pass, time()+3600); //logged in for 1day
			}
			else {
				if (!isset($_COOKIE["sauerserver"]))
					setcookie("sauerserver", null, -1);
				echo "Wrong Password!<br>";
				login();
			}
		}
		continue;
	}
	
	if ($log[$i] == "</server>") {
		$i++;
		$m = explode(chr(32), $log[$i]);
		$map = $m[0];
		$timeleft = $m[1]; 
		$gamemode = $m[2].$m[3];
		continue;
	}
	
	if ($log[$i] == "</score>") {
		$i++;
		$m = explode(chr(32), $log[$i]);
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
		$m = explode(chr(32), $log[$i]);	
		if ($m[6] == "spectator")
			$spec = "<a href=\"admin.php?exec=unspec $m[5]\">Unspectate</a>";
		else
		    $spec = "<a href=\"admin.php?exec=spec $m[5]\">Spectate</a>\n";
		
		if ($m[7] == "admin")
			$admin = "<a href=\"admin.php?exec=setpriv $m[5] none\">Take-Admin</a>";
		else
		    $admin = "<a href=\"admin.php?exec=setpriv $m[5] admin\">Give-Admin</a>";
				
		$player = "<tr><td><b><a onClick=\"newwindow(900, 620,'chatlog.php?name=$m[0]')\">".htmlspecialchars($m[0])."($m[5])</a></b></td><td><a onClick=\"newwindow(900, 
620,'findplayers.php?country=$m[10] $m[11]')\">$m[10] $m[11]</a></td><td><a onClick=\"newwindow(900, 620,'http://www.geoiptool.com/en/?IP=$m[9]')\">$m[9]</a></td><td>$m[8] 
ms</td><td>$m[1]</td><td>$m[2]</td><td>$m[3]</td><td>$m[4]</td><td>$admin</td><td>$spec</td><td><a href=\"admin.php?exec=kick $m[5]\">Kick</a></td></tr>\n";
		$players[] = $player;
		$frags[] = $m[1];
		$playercount++;
	}
}

if ($_POST["submit"] == "change map") {
	$command = 'changemap '.$_POST["smode"].' '.$_POST["smap"];
	if(www($command)) {
		html(0, $sname);
		echo 'changed succesfully the map';
	}
	else {
		html(2, $sname);
		echo 'there was a error while changing map'."\n";
		echo '<br>Command:'.$command;
	}
	echo '<br>';
	die("..going back");
}

if (isset($_REQUEST["exec"])) {
	$command = $_REQUEST["exec"];
	if(www($command)) {
		html(0, $sname);
		echo 'command was successfully executed'."\n";
	}
	else {
		html(2, $sname);
		echo 'there was a error while executing the command'."\n";
		echo '<br>Command:'.$command."\n";
	}
	echo '<br>';
	die("..going back");
}

html(5, $sname);

echo "<h2>Mode: $gamemode Map: <a onMouseOver=\"newwindow(280, 280,'inc/maps/$map.jpg')\">$map</a> Players: $playercount Timeleft: $timeleft Minute(s)</h2>\n";
echo '<form action="admin.php" method="post">Mode: <input type="text" name="smode" onClick="javascript:this.form.smode.focus();this.form.smode.select();" value="'.$gamemode.'"> Map: <input 
type="text" name="smap" onClick="javascript:this.form.smap.focus();this.form.smap.select();" value="'.$map.'"> <input type="submit" name="submit" value="change map"></form>'."\n";
echo '<form action="admin.php" method="post">Command: <input type="text" name="exec"> <input type="submit" value="execute"></form>'."\n";
echo '<table border="1">'."\n";
echo '<tr><td>Name</td><td>Country</td><td>IP</td><td>Ping</td><td>Frags</td><td>Deaths</td><td>Acc</td><td>Team</td><td colspan="3" align="center">Admin</td></tr>'."\n";

function BubbleSort($sort_array, $frags) { 
  for ($i = 0; $i < sizeof($sort_array); $i++){ 
    for ($j = $i + 1; $j < sizeof($sort_array); $j++){ 
        if ($frags[$i] < $frags[$j]){ 
          $tmp = $sort_array[$i]; 
          $sort_array[$i] = $sort_array[$j]; 
          $sort_array[$j] = $tmp; 
        } 
     } 
  } 
  return $sort_array; 
} 

$players = BubbleSort($players, $frags);
for ($i = 0; $i < count($players); $i++) {
	echo $players[$i];
}

	
	
echo '</table></div>'."\n";

function contains($var, $text) {
	if (strlen(str_replace($text, null, $var)) < strlen($var)) 
		return true;
	else 	
		return false;
}

?>
</body>
</html>

