<?php
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
	echo '<p>This page was created in ' .round($totaltime,2). ' seconds using 2 querys.</p>';
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
function build_pager ($page) {
	global $dbh;
	$count = $dbh->query("
	select COUNT(*)
	from
	        (select name,
	                frags,
	                count(name) as TotalGames
	        from players
	                inner join matches on players.match_id=matches.id
	                inner join ctfplayers on players.id=ctfplayers.player_id
	        where matches.datetime > (date(\"now\",\"".$_SESSION['querydate']."\"))  and frags > 0 group by name)
	where TotalGames > ". $_SESSION['MinimumGames']."
	");
	$rows = $count->fetchColumn();
	$pages = ( ceil($rows / 100) );
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
	print overlib("Filtering in affect<br />Filter MinimumGames <font color=white>".$_SESSION['MinimumGames']."</font><br /> Filter NoFrags","$rows results");
	return $pages;
}
function check_get () {
	switch ($_GET['querydate']) {
	        case "day":
	                $_SESSION['querydate'] = "start of day";
	                $_SESSION['MinimumGames'] = "0";
	        break;
	        case "week":
	                $_SESSION['querydate'] = "-7 days";
	                $_SESSION['MinimumGames'] = "1";
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
	        if ( ! $_SESSION['querydate'] ) { $_SESSION['querydate'] = "start of month";  $_SESSION['MinimumGames'] = "4"; }
	}
	
	if ( $_GET['page'] >= 2 ) {
	        $_SESSION['paging'] = ( ($_GET['page'] * 100) - 100 +1 );
	} else { $_SESSION['paging'] = 0; }
	
	if ( $_GET['orderby'] ) {
	        // Input Validation
	        if (preg_match("/(AgressorRating|DefenderRating|Kpd|Accuracy|TotalGames)/i", $_GET['orderby']) ) {
	                $_SESSION['orderby'] = $_GET['orderby'];
	        } else {
	                $_SESSION['orderby'] = "AgressorRating";
	        }
	} else { if (! $_SESSION['orderby'] ) { $_SESSION['orderby'] = "AgressorRating";} }
}
function stats_table () {
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
        where matches.datetime > date(\"now\",\"".$_SESSION['querydate']."\") and frags > 0 group by name order by ". $_SESSION['orderby']." desc)
where TotalGames > ". $_SESSION['MinimumGames'] ." limit ".$_SESSION['paging'].",100 ;

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
	                print "
	                        <tr onmouseover=\"this.className='highlight'\" onmouseout=\"this.className=''\">
	                                <td>$row[name]</td>
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
?>
