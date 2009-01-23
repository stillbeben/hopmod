<?php
function select_columns($var)
{
	global $column_list;
        if (!preg_match("/($column_list)/", $var['name']) ) { return ($var & 1); }
}
function column_wrapper($array, $filter) {  // Wrapper for select_columns
	if (! $filter ) { return $array; }
	global $column_list;
	$column_list = $filter;
	$filtered_array = array_filter($array, "select_columns");
	$column_list = "";
	return $filtered_array;
}
function serverDetails() {

// Pull Variables from Running Hopmod Server
	global $stats_db_filename;
	global $server_title;
	$stats_db_filename = GetHop("value absolute_stats_db_filename");
	if ( ! isset($stats_db_filename) ) { $stats_db_filename = "../scripts/stats/data/stats.db"; } //Attempt a reasonable guess
	$server_title = GetHop("value servername");
	if ( ! isset($server_title) ) { $server_title = "HOPMOD Server";} //Set it to something

}

function count_rows($query) {
	global $dbh;
	$count = $dbh->query($query) or die(print_r($dbh->errorInfo()));
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
function overlib($overtext,$heading = "") {
        print "<a  href=\"javascript:void(0);\" onmouseover=\"return overlib('$overtext');\" onmouseout=\"return nd();\">$heading</a>" ;
}
function overlib2($overtext,$heading = "") {
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
	$count = $dbh->query($query) or die(print_r($dbh->errorInfo()));
	$rows = $count->fetchColumn();
	$pages = ( ceil($rows / $rows_per_page) );
	print "<div style=\"float: right \" id=\"pagebar\">";
	if ( ! isset($page) ) { $page = 1; }
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
	print "</div>";
}
function check_get () {
	global $rows_per_page;
	switch ($_GET['querydate']) {
	        case "day":
	                $_SESSION['querydate'] = "day";
	                $_SESSION['MinimumGames'] = "1";
	        break;
	        case "week":
	                $_SESSION['querydate'] = "week";
	                $_SESSION['MinimumGames'] = "2";
	        break;
	        case "month":
	                $_SESSION['querydate'] = "month";
	                $_SESSION['MinimumGames']  = "2";
	        break;
	        case "year":
	                $_SESSION['querydate'] = "year";
	                $_SESSION['MinimumGames'] = "3";
	        break;
	default:
	        if ( ! isset($_SESSION['querydate']) ) { $_SESSION['querydate'] = "month"; }
		if ( ! isset($_SESSION['MinimumGames']) ) { $_SESSION['MinimumGames'] = 4; }
	}
	
	if ( $_GET['page'] >= 2 ) {
	        $_SESSION['paging'] = ( ($_GET['page'] * $rows_per_page) - $rows_per_page +1 );
	} else { $_SESSION['paging'] = 0; }
	
	if ( isset($_GET['orderby']) ) {
	        // Input Validation
	        if (preg_match("/(AgressorRating|DefenderRating|Kpd|Accuracy|TotalGames)/i", $_GET['orderby']) ) {
	                $_SESSION['orderby'] = $_GET['orderby'];
	        } else {
	                $_SESSION['orderby'] = "AgressorRating";
	        }
	} else { if (! isset($_SESSION['orderby']) ) { $_SESSION['orderby'] = "AgressorRating";} }
	if ( isset($_GET['name']) ) { $_SESSION['name'] = $_GET['name']; }
}
function stats_table ($query = "NULL" ,$exclude_columns = "NULL"){
	global $dbh;
	global $column_list; 
	global $rows_per_page;


//Table options
$stats_table = array (
    array("name" => "Name", "description" => "Players Nick Name", "column" => "name"),
    array("name" => "Country", "description" => "Players Country", "column" => "ipaddr"),
    array("name" => "CTF Agressor Rating", "description" => "Average Scores per Game + Average flag Pickups", "column" => "AgressorRating"),
    array("name" => "CTF Defender Rating", "description" => "Average Defends(kill flag carrier) per Game + Average flag returns", "column" => "DefenderRating"),
    array("name" => "FlagsDefended", "description" => "How many times you killed a flag carrier", "column" => "TotalDefended"),
    array("name" => "Frags Record", "description" => "The most frags ever acheived in one game", "column" => "MostFrags"),
    array("name" => "Total Frags", "description" => "The total number of frags for all games", "column" => "TotalFrags"),
    array("name" => "Total Deaths", "description" => "The total number of deaths for all games", "column" => "TotalDeaths"),
    array("name" => "Accuracy", "description" => "The percentage of shots fired that resulted in a frag", "column" => "Accuracy"),
    array("name" => "KpD", "description" => "The number of frags made before being killed", "column" => "Kpd"),
    array("name" => "TK", "description" => "The number of times a team member was fragged", "column" => "TotalTeamkills"),
    array("name" => "Games", "description" => "The total number of games played", "column" => "TotalGames"),
);
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
                round((0.0+(sum(scored)+sum(pickups)))/count(ctfplayers.player_id),2) as AgressorRating,
                round((0.0+(sum(defended)+sum(returns)))/count(ctfplayers.player_id),2) as DefenderRating
        from players
                inner join matches on players.match_id=matches.id
                outer left join ctfplayers on players.id=ctfplayers.player_id

        where matches.datetime between ".$_SESSION['start_date']." and ".$_SESSION['end_date']." and frags > 0 group by name order by ". $_SESSION['orderby']." desc)
where TotalGames >= ". $_SESSION['MinimumGames'] ." limit ".$_SESSION['paging'].",$rows_per_page ;

";
	if ( $query !="NULL") { $sql = $query; }
	$result = $dbh->query($sql) or die(print_r($dbh->errorInfo()));
	$gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);
?>
<table align="center" cellpadding="0" cellspacing="0" id="hopstats" class="tablesorter">
        <thead>
        <tr>
<?php
	foreach (column_wrapper($stats_table, $exclude_columns) as $column) { print "<th>";overlib($column['description'], $column['name']); print "</th>"; }
	print "<tr></thead><tbody>";
	foreach ($result as $row)
	{
	                $country = geoip_country_name_by_addr($gi, $row['ipaddr']);
	                $code = geoip_country_code_by_addr($gi, $row['ipaddr']);
	                if (isset($code)) {
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
					foreach (column_wrapper($stats_table, "Name|Country|$exclude_columns") as $column) {
						print "<td>".$row[$column['column']]."</td>";
					}
	                print "
	                        </tr>";
	        $flag_image ="";
	}
// Close db handle
print "</table>";
}

function match_table ($match) {
        global $dbh;
        $sql3 = "
select 
	datetime,
	duration,
	mapname,
	gamemode,
	players
from matches 
where id = '$match' 

";
$result = $dbh->query($sql3) or die(print_r($dbh->errorInfo()));


        $gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);
	$row = $result->fetch(PDO::FETCH_OBJ)
// Close db handle
?>

<div align="left" id="content"><h1>Match details</h1>
<div style="width:600px">
<div style="float:right; border:4px ridge grey; "><img src='images/maps/<?php print $row->mapname; ?>.jpg' /></div>
<h2></h2>
<table cellpadding="0" cellspacing="1">
<tr>
        <td class="headcol">Server</td>
        <td>-={Server}=-</td>
</tr>
<tr>
        <td style="width:100px;" class="headcol">Date/Time</td>
        <td><?php print date(" g:i A | jS M Y",$row->datetime); ?></td>
</tr>
<tr>
        <td class="headcol">Duration</td>
        <td><?php print $row->duration ?></td>
</tr>
<tr>
        <td class="headcol">Map</td>
        <td><?php print $row->mapname ?></td>
</tr>
<tr>
        <td class="headcol">Mode</td>
        <td><?php print $row->gamemode ?></td></tr>

</div>
<tr>
        <td class="headcol">Players</td>
        <td><?php print $row->players ?></td></tr>

</div>
</table>
</div></div>
<h2>Players</h2>
<?php
}

?>
