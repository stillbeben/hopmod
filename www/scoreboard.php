<?php
//Page Benchmark Start
$mtime = microtime(); 
$mtime = explode(' ', $mtime); 
$mtime = $mtime[1] + $mtime[0]; 
$starttime = $mtime; 

session_start();
include("includes/geoip.inc");
include("includes/hopmod.php");
if ( $_GET['querydate'] ) {
	// Input Validation
	if (! preg_match("(day|month|year|week)", $_GET['querydate']) ) { 
		$_SESSION['querydate'] = "start of month"; 
	} else {
		if ( $_GET['querydate'] == "week" ) {
			$_SESSION['querydate'] = "-7 days";
		} else {
			$_SESSION['querydate'] = "start of ".$_GET['querydate'];
		}
	}
} else { if (! $_SESSION['querydate'] ) { $_SESSION['querydate'] = "start of month"; } }
$querydate = $_SESSION['querydate'];

switch ($querydate) {
case "start of day":
    $MinimumGames = "0";
    break;
case "-7 days":
    $MinimumGames = "1";
    break;
case "start of month":
    $MinimumGames = "4";
    break;
case "start of year":
    $MinimumGames = "9";
    break;
}



if ( $_GET['page'] >= 2 ) {
	$paging = ( ($_GET['page'] * 100) - 100 +1 );
} else { $paging = 0; }

if ( $_GET['orderby'] ) {
	// Input Validation
	if (preg_match("/(AgressorRating|DefenderRating|Kpd|Accuracy|TotalGames)/i", $_GET['orderby']) ) {
		$_SESSION['orderby'] = $_GET['orderby']; 
	} else {
		$_SESSION['orderby'] = "AgressorRating";	
	}
} else { if (! $_SESSION['orderby'] ) { $_SESSION['orderby'] = "AgressorRating";} }


// Setup Geoip for location information.
$gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);

// Pull Variables from Running Hopmod Server
$stats_db_filename = GetHop("value absolute_stats_db_filename");
if ( ! $stats_db_filename ) { $stats_db_filename = "../scripts/stats/data/stats.db"; } //Attempt a reasonable guess
$server_title = GetHop("value title");
if ( ! $server_title ) { $server_title = "HOPMOD Server";} //Set it to something


// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($stats_db_filename);


// Setup main sqlite query.
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
        where matches.datetime > date(\"now\",\"$querydate\") and frags > 0 group by name order by ". $_SESSION['orderby']." desc)
where TotalGames > $MinimumGames limit $paging,100 ;

";
$count = $dbh->query("
select COUNT(*) 
from
 	(select name,
		frags,
		count(name) as TotalGames
	from players
                inner join matches on players.match_id=matches.id
                inner join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime > (date(\"now\",\"$querydate\"))  and frags > 0 group by name)
where TotalGames > $MinimumGames;

");
$result = $dbh->query($sql);
$rows = $count->fetchColumn();

?>
<html>
<head>
	<title><?php print $server_title; ?> scoreboard</title>
	<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
	<script type="text/javascript" src="js/jquery-latest.js"></script>
	<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
	<script type="text/javascript" src="js/jquery.uitablefilter.js"></script>
	<script type="text/javascript" src="js/hopstats.js"></script>
	<link rel="stylesheet" type="text/css" href="style.css" />
</head>
<body>

<ul align=right id="sddm">
    <li><a href="#" 
        onmouseover="mopen('m1')" 
        onmouseout="mclosetime()">Ordered by <?php print "<font color='white'>". $_SESSION['orderby'] ."</font>";?> </a>
        <div id="m1" 
            onmouseover="mcancelclosetime()" 
            onmouseout="mclosetime()">
        <a href="?orderby=Kpd">Kpd</a>
        <a href="?orderby=AgressorRating">Agressor Rating</a>
        <a href="?orderby=DefenderRating">Defender Rating</a>
        <a href="?orderby=Accuracy">Accuracy</a>
        <a href="?orderby=TotalGames">Total Games</a>
        </div>
    </li>
</ul>
<noscript><div class="error">This page uses JavaScript for table column sorting and producing an enhanced tooltip display.</div></noscript>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000"></div>
<h1><?php print "$server_title "; print "$month"; ?> Scoreboard</h1>

<div id="filter-panel">
<span class="filter-form">

Limit to this [ <a href="?querydate=day" <?php if ( $_SESSION['querydate'] == "start of day" ) { print "class=selected"; } ?>>DAY</a> | 
<a href="?querydate=week" <?php if ( $_SESSION['querydate'] == "-7 days" ) { print "class=selected"; } ?>>WEEK</a> | 
<a href="?querydate=month" <?php if ( $_SESSION['querydate'] == "start of month" ) { print "class=selected"; } ?> >MONTH</a> | 
<a href="?querydate=year" <?php if ( $_SESSION['querydate'] == "start of year" ) { print "class=selected"; } ?>>YEAR</a> ]</span>


<span class="filter-form"><form id="filter-form">Name Filter: <input name="filter" id="filter" value="" maxlength="30" size="30" type="text"></form></span>
</div>

<div id="pagebar" >
<?php

$pages = ( round($rows / 100 + 1) );
if ( ! $_GET['page'] ) { $_GET['page'] = 1; }
if ( $_GET['page'] <= "1" or $_GET['page'] > $pages ) {
        print "<a>Prev &#187;</a>";
	$_GET['page'] == "1";
} else {
        $nextpage = ($_GET['page'] - 1);
        print "<a href=\"?page=$nextpage\" >Prev &#171;</a>";
}

for ( $counter = 1; $counter <= $pages; $counter++) {
	?>

	<a href="?page=<?php print $counter ?>" <?php if ($counter == $_GET['page']) { print " class=selected";} ?> ><?php print $counter ?></a>

	<?php
}
if ( $_GET['page'] >= $pages or $_GET['page'] < "1" ) { 
	print "<a>Next &#187;</a>";
	$_GET['page'] == $pages;
} else {
	$nextpage = ($_GET['page'] + 1);
	print "<a href=\"?page=$nextpage\" >Next &#187;</a>";
}
?>
</div>

<table align="center" cellpadding="0" cellspacing="0" id="hopstats" class="tablesorter">
	<thead>
	<tr>
		<th><?php overlib("Player Name") ?>Name</a></th>
		<th><?php overlib("Players Country") ?>Country</a></th>
		<th><?php overlib("Average Scores per Game + Average flag Pickups") ?>Agressor Rating</a></th>
		<th><?php overlib("Average Defends(kill flag carrier) per Game + Average flag returns") ?>Defender Rating</a></th>
		<th><?php overlib("Flages Defended") ?>Flags Defended</a></th>
		<th><?php overlib("Highest Frags Recorded for 1 game") ?>Frags Record</a></th>
		<th><?php overlib("Total Frags Ever Recorded") ?>Total Frags</a></th>
		<th><?php overlib("Total Deaths") ?>Total Deaths</a></th>
		<th><?php overlib("Accuracy %") ?>Accuracy (%)</a></th>
		<th><?php overlib("Kills Per Death") ?>Kpd</a></th>
		<th><?php overlib("Team Kills") ?>TK</a></th>
		<th><?php overlib("Total Number of Games Played") ?>Games</a></th>
	</tr>
	</thead>
	<tbody>
<?php
//Build table data

#echo "-------Offset $paging----Page".$_GET['page']." ---Pages $pages----Rows $rows ------- Querydate $querydate ------$orderby";


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
				<td><?php overlib($country); print $flag_image ?></a></td>
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
$dbh = null;
?>
</tbody>
</table>
<div class="footer">
<span id="cdate">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
<?php 
//Page Benchmark End
$mtime = microtime(); 
$mtime = explode(" ", $mtime); 
$mtime = $mtime[1] + $mtime[0]; 
$endtime = $mtime; 
$totaltime = ($endtime - $starttime); 
echo '<p>This page was created in ' .round($totaltime,2). ' seconds using 2 querys.</p>'; 
?>
</div>

</body>
</html>
