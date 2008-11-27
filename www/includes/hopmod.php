<?php

function count_rows($query) {
	global $dbh;
	$count = $dbh->query($query);
	return $count->fetchColumn();
	
}
function startbench() {
	global $starttime;
	$mtime = microtime();
	$mtime = explode(' ', $mtime);
	$mtime = $mtime[1] + $mtime[0];
	$starttime = $mtime;
	return $starttime;
}
function stopbench() {
	global $starttime;
	$mtime = microtime();
	$mtime = explode(" ", $mtime);
	$mtime = $mtime[1] + $mtime[0];
	$endtime = $mtime;
	$totaltime = ($endtime - $starttime);
?>
<div class="footer">
<span id="date">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
<?php echo '<p>This page was created in ' .round($totaltime,2). ' seconds using 2 querys.</p>'; ?>
</div>
<?php
}
function GetHop($cubescript) {
        $content_length = strlen($cubescript);
        $headers= "POST /serverexec HTTP/1.0\r\nContent-type: text/cubescript\r\nHost: 127.0.0.1:7894\r\nContent-length: $content_length\r\n\r\n";
        $fp = fsockopen("127.0.0.1:7894");
        if (!$fp) return false;
        fputs($fp, $headers);
        fputs($fp, $cubescript);
        $ret = "";
        while (!feof($fp)) {
                $ret = fgets($fp, 1024);
        }
        fclose($fp);
        return $ret;
}
function overlib($overtext,$heading) {
        print "<a  href=\"javascript:void(0);\" onmouseover=\"return overlib('$overtext');\" onmouseout=\"return nd();\">$heading</a>" ;
}
function overlib2($overtext,$heading) {
        return "<a  href=\"javascript:void(0);\" onmouseover=\"return overlib('$overtext');\" onmouseout=\"return nd();\">$heading</a>"
;
}
function setup_pdo_statsdb($stats_db_filename) {
	try {
	        $dbh = new PDO("sqlite:$stats_db_filename");
	}
	catch(PDOException $e)
	{
	        echo $e->getMessage();
	}
	return $dbh;
}
function build_pager ($page, $query) {
	// current_page query link enable filtering display
	global $dbh;
	global $rows_per_page;
	$count = $dbh->query($query);
	$rows = $count->fetchColumn();
	$pages = ( ceil($rows / $rows_per_page) );
	if ( ! $page ) { $page = 1; }
	if ( $page <= "1" or $page > $pages ) {
	        print "<a>Prev &#187;</a>";
	        $page == "1";
	} else {
	        $nextpage = ($page - 1);
	        print "<a href=\"?page=$nextpage\" >Prev &#171;</a>";
	}
	
	for ( $counter = 1; $counter <= $pages; $counter++) {
	        ?>
	
	        <a href="?page=<?php print $counter ?>" <?php if ($counter == $page) { print " class=selected";} ?> ><?php print $counter ?></a>
	
	        <?php
	}
	if ( $page >= $pages or $page < "1" ) {
	        print "<a>Next &#187;</a>";
	        $page == $pages;
	} else {
	        $nextpage = ($page + 1);
	        print "<a href=\"?page=$nextpage\" >Next &#187;</a>";
	}
	print overlib("Filtering in affect<br />Filter MinimumGames <font color=white>".$_SESSION['MinimumGames']."</font>Filter NoFrags","$rows results");
}
function check_get () {
	global $rows_per_page;
	switch ($_GET['querydate']) {
	        case "day":
	                $_SESSION['querydate'] = "start of day";
	                $_SESSION['MinimumGames'] = "1";
	        break;
	        case "week":
	                $_SESSION['querydate'] = "-7 days";
	                $_SESSION['MinimumGames'] = "2";
	        break;
	        case "month":
	                $_SESSION['querydate'] = "start of month";
	                $_SESSION['MinimumGames']  = "4";
	        break;
	        case "year":
	                $_SESSION['querydate'] = "start of year";
	                $_SESSION['MinimumGames'] = "9";
	        break;
	default:
	        if ( ! $_SESSION['querydate'] ) { $_SESSION['querydate'] = "start of day"; }
		if ( ! $_SESSION['MinimumGames'] ) { $_SESSION['MinimumGames'] = 1; }
	}
	
	if ( $_GET['page'] >= 2 ) {
	        $_SESSION['paging'] = ( ($_GET['page'] * $rows_per_page) - $rows_per_page +1 );
	} else { $_SESSION['paging'] = 0; }
	
	if ( $_GET['orderby'] ) {
	        // Input Validation
	        if (preg_match("/(AgressorRating|DefenderRating|Kpd|Accuracy|TotalGames)/i", $_GET['orderby']) ) {
	                $_SESSION['orderby'] = $_GET['orderby'];
	        } else {
	                $_SESSION['orderby'] = "AgressorRating";
	        }
	} else { if (! $_SESSION['orderby'] ) { $_SESSION['orderby'] = "AgressorRating";} }
	if ( $_GET['name'] ) { $_SESSION['name'] = $_GET['name']; }
}
function stats_table ($query = "null") {
	global $dbh;
	$sql = "
select *
from
        (select name,
                ipaddr,
                sum(pickups) as TotalPickups,
                sum(drops) as TotalDrops,
                sum(scored) as TotalScored,
                sum(teamkills) as TotalTeamkills,
                sum(defended) as TotalDefended,
                max(frags) as MostFrags,
                sum(frags) as TotalFrags,
                sum(deaths) as TotalDeaths,
                count(name) as TotalGames,
                round((0.0+sum(hits))/(sum(hits)+sum(misses))*100) as Accuracy,
                round((0.0+sum(frags))/sum(deaths),2) as Kpd,
                round((0.0+(sum(scored)+sum(pickups)))/count(name),2) as AgressorRating,
                round((0.0+(sum(defended)+sum(returns)))/count(name),2) as DefenderRating
        from players
                inner join matches on players.match_id=matches.id
                inner join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime > strftime(\"%s\",\"now\",\"".$_SESSION['querydate']."\") and frags > 0 group by name order by ". $_SESSION['orderby']." desc)
where TotalGames >= ". $_SESSION['MinimumGames'] ." limit ".$_SESSION['paging'].",100 ;

";
	if (! $query = "null") { $sql = $query; }
	$result = $dbh->query($sql);
	$gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);
	foreach ($result as $row)
	{
	                $country = geoip_country_name_by_addr($gi, $row['ipaddr']);
	                $code = geoip_country_code_by_addr($gi, $row['ipaddr']);
	                if ($code) {
	                        $code = strtolower($code) . ".png";
	                        $flag_image = "<img src=images/flags/$code />";
	                }
	                print "
	                        <tr onmouseover=\"this.className='highlight'\" onmouseout=\"this.className=''\">
					<td><a href=\"player.php?name=$row[name]\">$row[name]</a></td>
	                                ";
	                                ?>
	                                <td><?php overlib($country,$flag_image);?></td>
	                                <?php
	                print "
	                                <td>$row[AgressorRating]</td>
	                                <td>$row[DefenderRating]</td>
	                                <td>$row[TotalDefended]</td>
	                                <td>$row[MostFrags]</td>
	                                <td>$row[TotalFrags]</td>
	                                <td>$row[TotalDeaths]</td>
	                                <td>$row[Accuracy]</td>
	                                <td>$row[Kpd]</td>
	                                <td>$row[TotalTeamkills]</td>
	                                <td>$row[TotalGames]</td>
	                        </tr>";
	        $flag_image ="";
	}
// Close db handle
$dbh = null;
}

function match_table ($match) {
        global $dbh;
        $sql = "
select name,
	matches.datetime,
	matches.duration,
	matches.mapname,
	matches.gamemode,
	matches.demofile,
	matches.players,
	ipaddr,
	sum(frags) as frags,
	sum(deaths) as deaths,
	name,
	round((0.0+hits) / (hits+misses)*100) as Accuracy,
	round((0.0+frags/deaths),2) as Kpd
from players 
	inner join matches on players.match_id=matches.id
	outer left join ctfplayers on players.id=ctfplayers.player_id
where match_id = '$match' group by name order by frags desc
;

";
$result = $dbh->query($sql);


        $gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);
        foreach ($result as $row)
        {
                        $country = geoip_country_name_by_addr($gi, $row['ipaddr']);
                        $code = geoip_country_code_by_addr($gi, $row['ipaddr']);
                        if ($code) {
                                $code = strtolower($code) . ".png";
                                $flag_image = "<img src=images/flags/$code />";
                        }
			$country = overlib2($country,$flag_image);
                        $table .= "
                                <tr onmouseover=\"this.className='highlight'\" onmouseout=\"this.className=''\">
                                        <td><a href=\"player.php?name=$row[name]\">$row[name]</a></td>
                                        <td>$country</td>
                                        <td>$row[frags]</td>
                                        <td>$row[deaths]</td>
                                        <td>".($row[Accuracy]+0)."%</td>
                                        <td>".($row[Kpd]+0.0)."</td>
                                </tr>";
                $flag_image ="";
        }
// Close db handle
$dbh = null;
?>

<div align="left" id="content"><h1>Match details</h1>
<div style="width:600px">
<div style="float:right; border:4px ridge grey; "><img src='images/maps/<?php print $row['matches.mapname']?>.jpg' /></div>
<h2></h2>
<div class="box">
<table cellpadding="0" cellspacing="1">
<tr>
        <td class="headcol">Server</td>
        <td>-={Server}=-</td>
</tr>
<tr>
        <td style="width:100px;" class="headcol">Date/Time</td>
        <td><?php print date(" g:i A | jS M Y",$row['matches.datetime']); ?></td>
</tr>
<tr>
        <td class="headcol">Duration</td>
        <td><?php print $row['matches.duration'] ?></td>
</tr>
<tr>
        <td class="headcol">Map</td>
        <td><?php print $row['matches.mapname'] ?></td>
</tr>
<tr>
        <td class="headcol">Mode</td>
        <td><?php print $row['matches.gamemode'] ?></td></tr>

</div>
<tr>
        <td class="headcol">Players</td>
        <td><?php print $row['matches.players'] ?></td></tr>

</div>
<tr>
        <td class="headcol">Demo</td>
        <td><?php print $row['matches.demofile'] ?></td></tr>

</div>
</table>
<h2>Players</h2>
<table class="tablesorter" id="hopstats" cellpadding="0" cellspacing="1" style="width:200%">
<thead>
        <tr>
                <th>Name</th>
                <th>Country</th>
                <th>Frags</th>
                <th>Deaths</th>
                <th>Accuracy</th>
                <th>kpd</th>
        </tr>
</thead>
<?php
print $table;
print "</table></div></div>";
}
?>
