<?php

session_start();
include("includes/geoip.inc");
include("includes/hopmod.php");
$querydate = "month";
if ( $_GET['querydate'] ) {
	if (! preg_match('(day|week|month|year)', $_GET['querydate']) ) { $querydate = "month";} else { $querydate = $_GET['querydate']; }
}
if ( $_GET['showprofile'] ) {

}
// Setup Geoip for location information.
$gi = geoip_open("/usr/local/share/GeoIP/GeoIP.dat",GEOIP_STANDARD);

// Pull Variables from Running Hopmod Server
$stats_db_filename = GetHop("value absolute_stats_db_filename");
$server_title = GetHop("value title");

// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($stats_db_filename);

// Setup main sqlite query.
echo "-------------------------- $querydate";
$sql = "select name,
                ipaddr,
                sum(pickups) as TotalPickups,
                sum(drops) as TotalDrops,
                sum(scored) as TotalScored,
                sum(teamkills) as TotalTeamkills,
                sum(defended) as TotalDefended,
                max(frags) as MostFrags,
                sum(frags) as TotalFrags,
                sum(deaths) as TotalDeaths,
                count(name) as TotalMatches,
                round((0.0+sum(hits))/(sum(hits)+sum(misses))*100) as Accuracy,
                round((0.0+sum(frags))/sum(deaths),2) as Kpd,
                round((0.0+(sum(scored)+sum(pickups)))/count(name),2) as ASpG,
                round((0.0+(sum(defended)+sum(returns)))/count(name),2) as ADpG
        from players
                inner join matches on players.match_id=matches.id
                inner join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime > date(\"now\",\"start of $querydate\") group by name order by ASpG desc limit 300";



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
<noscript><div class="error">This page uses JavaScript for table column sorting and producing an enhanced tooltip display.</div></noscript>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000"></div>
<h1><?php print "$server_title "; print "$month"; ?> Scoreboard</h1>

<form id="filter-form">Filter: <input name="filter" id="filter" value="" maxlength="30" size="30" type="text"></form><br>
<div class="filter-form" align=left>Limit to this [ <a href="ctf.php?querydate=day">DAY</a> | <a href="ctf.php?querydate=week">WEEK</a> | <a href="ctf.php?querydate=month">MONTH</a> | <a href="ctf.php?querydate=year">YEAR</a> ]</div>
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
foreach ($dbh->query($sql) as $row)
{
	if ( $row['TotalFrags'] > 50 & $row['name'] != "unnamed") {
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
				<td>$row[ASpG]</td>
				<td>$row[ADpG]</td>
				<td>$row[TotalDefended]</td>
				<td>$row[MostFrags]</td>
				<td>$row[TotalFrags]</td>
				<td>$row[TotalDeaths]</td>
				<td>$row[Accuracy]</td>
				<td>$row[Kpd]</td>
				<td>$row[TotalTeamkills]</td>
				<td>$row[TotalMatches]</td>
        		</tr>";
	}
	$flag_image ="";
}
?>
</tbody>
</table>
<div class="footer">
<span id="cdate">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
</div>

</body>
</html>
