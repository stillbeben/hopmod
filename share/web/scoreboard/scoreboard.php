<?php
php_version_check();
// Config
$database["path"] = '/home/sauer/server/server_new/db/stats.sqlite';

$db = new SQLite3($database["path"]);

//if ($_GET["admin"] == 1) $admin = true;

if ($_GET["nick"] != NULL) {
	$results = $db->query('	
	select *, round(hits/(max(shots,1)+0.0)*100) as acc
	from playertotals
	where timeplayed >= 1 and name != "unnamed" and name != "bot"
	and name like \'%'.$_GET["nick"].'%\'
	order by name desc
	');
	while ($row = $results->fetchArray()) {
		$html .= '<b><a href="?show=player&player='.$row["name"].'">'.$row["name"].'</a></b> || ';
	}	
	echo substr($html, 0, strlen($html)-4);
	exit;
}

if ($_GET["show"] == NULL) { 
	if ($_GET["fplayer"] == NULL) header("Location: ?show=players"); 
	else header("Location: ?show=players&fplayer=".$_GET["fplayer"]);
	exit;
} // if no show argument is given, force to view players 

style();
if ($_GET["show"] == "games") show_games($db);
if ($_GET["show"] == "players") {
	if ($_GET["fplayer"] == NULL) show_players($db, $_GET["order"], $_GET["c"], NULL, NULL, $admin);
	else show_players($db, $_GET["order"], $_GET["c"], $_GET["fplayer"], true, $admin);
}
if ($_GET["show"] == "player") {
	show_player($db, $_GET["player"], $_GET["order"], $admin);
}

function find_player() {
	echo '<div align="center">';
	echo '<form action="?show=players" method="GET">';
	echo '<input type="text" onKeyPress="find_user(this.value)" onKeyUp="find_user(this.value)" name="fplayer">';
	echo '&nbsp;<input type="submit" value="Find Player">';
	echo '<div id="html"></div>';
	echo '</form>';
	echo '</div>';
}


function show_player($db, $player, $order, $admin) {
	$player = fix_back($player);
	$order = str_replace("'", null, str_replace('"', null, $order));
	$order = str_replace(chr(92), null, $order);
	if(!$order) $order = "timeplayed";
	$results = $db->query('	
	select *, round(hits/(max(shots,1)+0.0)*100) as acc
	from players 
	inner join games on games.id = players.game_id where players.name = "'.$player.'"
	and players.timeplayed >= 2 and gamemode != "coop edit"
	order by "'.$order.'" desc
	');
	echo '<div align="center">Player Name:  '.$player;
	echo '<table border="1" align="center" class="stats">';
	echo '
	<tr>
	<td><a href="?show=player&player='.$player.'&order=gametime">Time</a></td>
	<td><a href="?show=player&player='.$player.'&order=gamemode">Mode</a></td>
	<td><a href="?show=player&player='.$player.'&order=gamemap">Map</a></td>
	<td><a href="?show=player&player='.$player.'&order=frags">Frags</a></td>
	<td><a href="?show=player&player='.$player.'&order=deaths">Deaths</a></td>
	<td>Kpd</td>
	<td><a href="?show=player&player='.$player.'&order=acc">Acc</a></td>
	<td><a href="?show=player&player='.$player.'&order=suicides">Suicides</a></td>
	<td><a href="?show=player&player='.$player.'&order=teamkills">Teamkills</a></td>
	<td><a href="?show=player&player='.$player.'&order=hits">Hits</a></td>
	<td><a href="?show=player&player='.$player.'&order=shots">Shots</a></td>
	<td><a href="?show=player&player='.$player.'&order=timeplayed">Playtime</a></td>
	</tr>
	';
	
	while ($row = $results->fetchArray()) {
		$p[stat_day][$row["datetime"]][frags] += $row["frags"];
		$p[stat_day][$row["datetime"]][deaths] += $row["deaths"];
		
		echo '<tr>';
		echo '<td>';
		echo date("d.m.Y | H:i", $row["datetime"]);
		echo '</td>';
		echo '<td>';
		echo $row["gamemode"];
		echo '</td>';
		echo '<td>';
		echo '<div id="flag" align="left"><a href="maps/png/'.$row["mapname"].'.png"><img width="20" height="20" src="maps/'.$row["mapname"].'.jpg"></a> - '.$row["mapname"].'</div>';
		//echo $row["mapname"];
		echo '</td>';
		echo '<td>';
		echo $row["frags"];
		echo '</td>';
		echo '<td>';
		echo $row["deaths"];
		echo '</td>';
		echo '<td>';
		if (is_numeric($row["frags"]) && is_numeric($row["deaths"]) && $row["deaths"] > 0)
			echo round($row["frags"]/$row["deaths"], 2);
		else 
			echo 0;
		echo '</td>';
		echo '<td>';
		echo $row["acc"];
		echo '%</td>';
		echo '<td>';
		echo $row["suicides"];
		echo '</td>';
		echo '<td>';
		echo $row["teamkills"];
		echo '</td>';
		echo '<td>';
		echo $row["hits"];
		echo '</td>';
		echo '<td>';
		echo $row["shots"];
		echo '</td>';
		echo '<td>';
		echo duration($row["timeplayed"]);
		echo '</td>';
		echo '</tr>';
		$p[games]++;
		$p[time] += $row["timeplayed"];
		flush();
	}
	echo '</table>';
	echo '<b>Full Playtime</b>: '.duration($p[time]). ' <b>Games</b>: '.$p[games];
	echo '</div>';
}

function page_links($page, $pages, $order) {
	echo '<div align="center"><font face="Tahoma" size="-1">';
	if (($page+1) >= $pages)
		echo '<a href="?show=players&order='.$order.'&page='.($page-1).'"><b>back</b></a>&nbsp';
	for ($i = 0; $i <= $pages; $i++) {
		if ($page == $i)
			echo '<a href="?show=players&order='.$order.'&page='.$i.'"><b>'.$i.'</b></a>&nbsp;';
		else
			echo '<a href="?show=players&order='.$order.'&page='.$i.'">'.$i.'</a>&nbsp;';
	}
	if (($page+1) <= $pages)
		echo '<a href="?show=players&order='.$order.'&page='.($page+1).'"><b>next</b></a>&nbsp';
	echo '</font></div>';
}

function show_players($db, $order,$country = NULL, $player = NULL, $norank = FALSE, $admin) {
	$max_per_page = 100;
	
	$order = str_replace("'", null, str_replace('"', null, $order));
	$order = str_replace(chr(92), null, $order);
	if(!$order) $order = "frags";
	if($country) $country = "and country = '".$country."'";
	if($player) $player = "and name like '%".$player."%'";
	
	$results = $db->query('
	select *, round(hits/(max(shots,1)+0.0)*100) as acc
	from playertotals
	where timeplayed >= 1 and name != "unnamed" and name != "bot" '.$country.' '.$player.'
	order by '.$order.' desc
	');
	echo '
	<table border="1" align="center" class="stats">
		<tr>';
			if (!$norank) {	echo '		<td>Rank</td>'; }
			echo '<td><a href="?show=players&order=country&fplayer='.$_GET["fplayer"].'">Country</a></td>
			<td><a href="?show=players&order=name&fplayer='.$_GET["fplayer"].'">Name</a></td>';
			if ($admin) echo '<td>IP</td>';
			echo '
			<td><a href="?show=players&order=frags&fplayer='.$_GET["fplayer"].'">Frags</a></td>
			<td><a href="?show=players&order=deaths&fplayer='.$_GET["fplayer"].'">Deaths</a></td>
			<td>Kpd</td>
			<td><a href="?show=players&order=acc&fplayer='.$_GET["fplayer"].'">Acc</a></td>
			<td><a href="?show=players&order=wins&fplayer='.$_GET["fplayer"].'">Wins</a></td>
			<td><a href="?show=players&order=losses&fplayer='.$_GET["fplayer"].'">Losses</a></td>
			<td><a href="?show=players&order=suicides&fplayer='.$_GET["fplayer"].'">Suicides</a></td>
			<td><a href="?show=players&order=teamkills&fplayer='.$_GET["fplayer"].'">Teamkills</a></td>
			<td><a href="?show=players&order=hits&fplayer='.$_GET["fplayer"].'">Hits</a></td>
			<td><a href="?show=players&order=shots&fplayer='.$_GET["fplayer"].'">Shots</a></td>
			<td><a href="?show=players&order=timeplayed&fplayer='.$_GET["fplayer"].'">Played Time</a></td>
		</tr>
	';
	if ($_GET["page"] == NULL) 
		$page = 0;
	elseif (is_numeric($_GET["page"]))
		$page = $_GET["page"];
	else 
		exit;
		
		
	$rank = ($page * $max_per_page);
	if ($page > 0) $rank -= 1;
	$begin = ($page * $max_per_page);
	$i = 0;
	while ($row = $results->fetchArray()) {	
		$i++;
	    if ($i > (($max_per_page * $page)+$max_per_page)) continue;
		if ($i < $begin) continue;
		$rank++;
		
		echo '<tr>';
		if (!$norank) {
			echo '<td>';
			echo $rank;
			echo '</td>';
		}
		echo '<td><div align="center" id="flag"><a href="?show=players&fplayer='.$_GET["fplayer"].'&page='.$page.'&c='.$row["country"].'"><img class="a" src="flags/'.strtolower($row["country"]).'.gif" alt="'.$row["country"].'" title="'.$row["country"].'"></a></div>';
		echo '</td>';
		echo '<td><a href="?show=player&player=';
		echo fix($row["name"]);
		echo '">';
		echo $row["name"];
		echo '</a></td>';
		if ($admin) {
			echo '<td>';
			echo " ".$row["ipaddr"];
			echo '</td>';
		}
		echo '<td>';
		echo $row["frags"];
		echo '</td>';
		echo '<td>';
		echo $row["deaths"];
		echo '</td>';
		echo '<td>';
		if (is_numeric($row["frags"]) && is_numeric($row["deaths"]) && $row["deaths"] > 0)
			echo round($row["frags"]/$row["deaths"], 2);
		else 
			echo 0;
		echo '</td>';
		echo '<td>';
		echo $row["acc"];
		echo '%</td>';
		echo '<td>';
		echo $row["wins"];
		echo '</td>';
		echo '<td>';
		echo $row["losses"];
		echo '</td>';
		echo '<td>';
		echo $row["suicides"];
		echo '</td>';
		echo '<td>';
		echo $row["teamkills"];
		echo '</td>';
		echo '<td>';
		echo $row["hits"];
		echo '</td>';
		echo '<td>';
		echo $row["shots"];
		echo '</td>';
		echo '<td>';
		echo duration($row["timeplayed"]);
		echo '</td>';
		echo '</tr>';
		echo "\n";
		flush();
	}
	echo '</table>';
	echo '<br>';
	find_player();
	page_links($page, round($i / $max_per_page, 0), $order);
}

function tr($str) {
	echo "	".$str."\n";
}

function td($str) {
	echo "		".$str;
}

function show_games($db) {
	$results = $db->query('SELECT * FROM games ORDER BY datetime DESC');

	echo '<table border="1" align="center">';
	echo '<tr><td>Time</td><td>Mode</td><td>Map</td><td>Players</td></tr>';

	while ($row = $results->fetchArray()) {
	  echo '<tr>';
	  echo '<td>'.date("d.m.Y | H:i", $row["datetime"]).'</td>';
	  echo '<td>'.$row["gamemode"].'</td>';
	  echo '<td>'.$row["mapname"].'</td>';
	  echo '<td>'.$row["players"].'</td>';
	  echo '</tr>';   
	}
	echo '</table>';
}

function print_time($n,$unit)
{
    if($n == 0) return "";
    else return $n . " " . ($n == 1 ? $unit : $unit . "s");
}

function duration($seconds)
{
    $years = floor($seconds / 31104000);
    $seconds -= $years * 31104000;
    
    $months = floor($seconds / 2592000);
    $seconds -= $months * 2592000;
    
    $days = floor($seconds / 86400);
    $seconds -= $days * 86400;
    
    $hours = floor($seconds / 3600);
    $seconds -= $hours * 3600;
    
    $minutes = floor($seconds / 60);
    $seconds -= $minutes * 60;
    
    if($years)
        return print_time($years,"year") . " " . print_time($months,"month");
    else if($months)
        return print_time($months,"month") . " " . print_time($days,"day");
    else if($days)
        return print_time($days,"day") . " " . print_time($hours,"hour");
    else if($hours)
        return print_time($hours,"hour") . " " . print_time($minutes,"minute");
    else if($minutes)
        return print_time($minutes,"minute") . " " . print_time($seconds,"sec");
    else
        return print_time($seconds,"sec");
}


function style() {
	?>
<html>
<head>
	<title>Sauerbraten Scoreboard</title>
	<style type="text/css">
		A:link { color: black; text-decoration: none; border:0px; }
		A:visited { color: black; text-decoration: none; border:0px; }
		A:hover { color: orange; border:0px;  }
		
		#flag img { border: 0px; }

		table.stats
		{text-align: center;
		font-family: Verdana, Geneva, Arial, Helvetica, sans-serif ;
		font-weight: normal;
		font-size: 11px;
		color: #fff;
		background-color: #666;
		border: 0px;
		border-collapse: collapse;
		border-spacing: 0px;}
		table.stats td
		{background-color: #CCC;
		color: #000;
		padding: 4px;
		text-align: left;
		border: 1px #fff solid;}
			table.stats td.hed
		{background-color: #666;
		color: #aaa;
		padding: 4px;
		text-align: left;
		border-bottom: 2px #fff solid;
		font-size: 12px;
		font-weight: bold;} 
	</style>
	<script type="text/javascript">
	function find_user(nick) {
		if (!nick) {
			document.getElementById('html').innerHTML="";
			return false;
		}
		get("?nick=" + nick, "ausgabe_html");
	}
			
	function ausgabe_html(users) {
		document.getElementById('html').innerHTML='<font face="Tahoma" size="-2">Matches: ' + users + '</font>';
		document.getElementById('html').style.visibility='visible';
	}
			
	function get(url, callback_function, return_xml){
		var http_request = false;	
		if (window.XMLHttpRequest) {
			http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml; charset= iso-8859-1');
		}
		}else if(window.ActiveXObject) {
			try {
				http_request = new ActiveXObject("Msxml2.XMLHTTP");
			}catch (e) {
				try {
					http_request = new ActiveXObject("Microsoft.XMLHTTP");
				} catch (e) {}
			}
		}
		if (!http_request) {
			eval(callback_function + '(404)');
			return false;
		}
		http_request.onreadystatechange = function() {
			if (http_request.readyState == 4) {
				if (http_request.status == 200) {
					if (return_xml) {
						eval(callback_function + '(http_request.responseXML)');
					} else {
						eval(callback_function + '(http_request.responseText)');
						StopTheClock();
					}
				} 
			}
		}
		http_request.open('GET', url, true);
		http_request.send(null);
	}
	</script>
</head>
<body bgcolor="#CCCCCC">
	<?php
}

function fix($str) {
	$str = str_replace("#", "%%1", $str);
	$str = str_replace("+", "%%2", $str);
	$str = str_replace(chr(92), '%%3', $str);
	$str = str_replace("'", '%%4', $str);
	return $str;
}

function fix_back($str) {
	$str = str_replace("%%1", "#", $str);
	$str = str_replace("%%2", "+", $str);
	$str = str_replace('%%3', chr(92), $str);
	$str = str_replace("%%4", "'", $str);
	$str = str_replace('"', NULL, $str);
	return $str;
}	

function php_version_check() {
	$version = phpversion();
	$x = $version[0]; // 5
	$x .= $version[2]; // 3
	$x .= $version[4]; // 0
	if ($x < 530) die("PHP >= 5.3.0RC1 is required");
}
?>
</body>
</html>