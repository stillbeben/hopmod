<?php
$stats_db_filename = exec("wget -o /dev/null -O /dev/stdout --timeout=5 --header \"Content-type: text/cubescript\" --post-data=\"value absolute_stats_db_filename\" http://127.0.0.1:7894/serverexec", $return);
if ( $return = 0  ) { echo "<font color=red>Error connecting to server for value absolute_stats_db_filename. Is the server running? Contact the administrator</font>"; }
try {
	$dbh = new PDO("sqlite:$stats_db_filename");
}
catch(PDOException $e)
{
	echo $e->getMessage();
}
$month = date("F");
$server_title = exec("wget -o /dev/null -O /dev/stdout --timeout=5 --header \"Content-type: text/cubescript\" --post-data=\"value title\" http://127.0.0.1:7894/serverexec");
?>
<html>
<head>
<title><?php print $server_title; ?> scoreboard</title>
<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
<script type="text/javascript" src="js/jquery-latest.js"></script>
<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
<script type="text/javascript" id="js">
$(document).ready(function()
       { 
                $("#hopstats").tablesorter( {sortList: [[0,0]]} ); 
        } 
);
</script>

<link rel="stylesheet" type="text/css" href="style.css" />
</head>
<body>
<noscript><div class="error">This page uses JavaScript for table colum sorting and producing an enhanced tooltip display.</div></noscript>
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000"></div>
<h1><?php print "$server_title "; print "$month"; ?> Scoreboard</h1>
<table align="center" cellpadding="0" cellspacing="0" id="hopstats" class="tablesorter">
	<thead>
	<tr>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Player Name');" onmouseout="return nd();">Name</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Average Scores per Game + Average flag Pickups');" onmouseout="return nd();">Agressor Rating</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Average Defends(kill flag carrier) per Game + Average flag returns');" onmouseout="return nd();">Defender Rating</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Flages Defended');" onmouseout="return nd();">Flags Defended</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Highest Frags Recorded for 1 game');" onmouseout="return nd();">Frags Record</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Total Frags Ever Recorded');" onmouseout="return nd();">Total Frags</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Total Deaths');" onmouseout="return nd();">Total Deaths</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Accuracy %');" onmouseout="return nd();">Accuracy (%)</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Kills Per Death');" onmouseout="return nd();">Kpd</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Team Kills');" onmouseout="return nd();">TK</a></th>
		<th><a href="javascript:void(0);" onmouseover="return overlib('Total Number of Games Played');" onmouseout="return nd();">Games</a></th>
	</tr>
	</thead>
	<tbody>
<?php
$sql = "select name,
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
	where matches.datetime > date(\"now\",\"start of month\") group by name order by ASpG desc limit 300";

foreach ($dbh->query($sql) as $row)
{
	if ( $row[TotalFrags] > 50 & $row[name] != "unnamed") {
        	print "
        		<tr onmouseover=\"this.className='highlight'\" onmouseout=\"this.className=''\">
				<td>$row[name]</td>
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
}
?>
</tbody>
</table>

<div class="footer">
<span id="cdate">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
</div>

</body>
</html>