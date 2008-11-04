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
<style type="text/css">
body
{
	background-color: #000000;
	color: #f0f0f0;
	text-align:center;
	font-family: arial;
}
a{color:#FF9900;}
a:visited{font-style:italic;}
table
{
	border-collapse:collapse;
}
table td,table th{border:solid 1px  #304860;}
table th{background-color:#183048;}
table th{padding:20px;}
table td{padding:5px; color:#ffffff;}
.highlight td{background-color: #302c28;}
.footer
{
	margin-top:20px;
	color: #a0a0a0;
	font-size:small;
}
</style>

</head>
<body>
<h1><?php print "$server_title "; print "$month"; ?> Scoreboard</h1>
<table align="center" cellpadding="0" cellspacing="0">
	<th>Name</th>
	<th>Flags Scored</th>
	<th>Flags Defended</th>
	<th>Frags Record</th>
	<th>Total Frags</th>
	<th>Total Deaths</th>
	<th>Accuracy (%)</th>
	<th>Kpd</th>

<?php
$sql = "select name,
		sum(scored) as TotalScored,
		sum(defended) as TotalDefended,
		max(frags) as MostFrags,
		sum(frags) as TotalFrags,
		sum(deaths) as TotalDeaths,
		round((0.0+sum(hits))/(sum(hits)+sum(misses))*100) as Accuracy,
		round((0.0+sum(frags))/sum(deaths),2) as Kpd
	from players
		inner join matches on players.match_id=matches.id
		inner join ctfplayers on players.id=ctfplayers.player_id
	where matches.datetime > date(\"now\",\"start of year\") group by name order by Kpd desc limit 100";

foreach ($dbh->query($sql) as $row)
{
	if ( $row[TotalFrags] > 50 ) {
        	print "
        		<tr onmouseover=\"this.className=\'highlight\'\" onmouseout=\"this.className=\'\'\">
				<td>$row[name]</td>
				<td>$row[TotalScored]</td>
				<td>$row[TotalDefended]</td>
				<td>$row[MostFrags]</td>
				<td>$row[TotalFrags]</td>
				<td>$row[TotalDeaths]</td>
				<td>$row[Accuracy]</td>
				<td>$row[Kpd]</td>
        		</tr>";
	}
}
?>

</table>
<div class="footer">
<span id="cdate">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
</div>
</body>
</html>
