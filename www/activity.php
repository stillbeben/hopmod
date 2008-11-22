<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

// Start session for session vars
session_start();

// Start page benchmark
startbench();

// Check for any http GET activity
check_get();
if (!$_SESSION['days']) { $_SESSION['days'] = 0;}
if ($_GET['select_day'] == "next") { $_SESSION['days'] = ($_SESSION['days'] + 1);header("location: activity.php");}
if ($_GET['select_day'] == "previous") { $_SESSION['days'] = ($_SESSION['days'] - 1);header("location: activity.php");}

$start_date = strtotime(($_SESSION['days']-1)." days");
$start_date = date("d F Y", $start_date);
$start_date = strtotime("$start_date");
$end_date = strtotime("+23 hours 59 minutes 59 seconds", $start_date); 


$day_matches = "
select matches.id as id,datetime,gamemode,mapname,duration,players
        from matches
        where matches.datetime between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  order by datetime desc 
";
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
        where matches.datetime between $start_date and $end_date and mapname != '' group by name order by Kpd desc limit 50)

";

// Pull Variables from Running Hopmod Server
$stats_db_filename = GetHop("value absolute_stats_db_filename");
if ( ! $stats_db_filename ) { $stats_db_filename = "stats.db"; } //Attempt a reasonable guess
$server_title = GetHop("value title");
if ( ! $server_title ) { $server_title = "HOPMOD Server";} //Set it to something

// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($stats_db_filename);

$count_day_matches = count_rows("
select count(*)
        from matches
        where matches.datetime between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  order by datetime desc
");
$player_count = count_rows("
select count(*) 
from
        (select name
        from players
                inner join matches on players.match_id=matches.id
                inner join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime between $start_date and $end_date and mapname != '' group by name )


");

?>

<html>
<head>
	<title><?php print $server_title; ?> scoreboard</title>
	<script type="text/javascript" src="js/overlib.js"><!-- overLIB (c) Erik Bosrup --></script>
	<script type="text/javascript" src="js/jquery-latest.js"></script>
	<script type="text/javascript" src="js/jquery.tablesorter.js"></script>
	<script type="text/javascript" src="js/jquery.uitablefilter.js"></script>
	<script type="text/javascript" src="js/hopstats.js"></script>
	<link rel="stylesheet" type="text/css" href="css/style.css" />
</head>














<body>





<br />
<h1>Daily Activity for <?php print date(" jS M Y",$start_date); ?><span style="font-style:italic; font-size:1.1em"><?php print $date; ?></span></h1>





<div id=container>

<div style='width: 200px; position: relative;'>
<a href="activity.php?select_day=previous">&#171; Previous day</a>
| <a href="activity.php?select_day=next">Next day &#187;</a>

        <h2>Matches (<?php print $count_day_matches; ?>)</h2>
</div>

<div id=leftColumn>






<?php



foreach ($dbh->query($day_matches) as $row){

?>


<li class="entrylist">

<a href="match.php?id=<?php print $row['id'] ?>">
    <span><?php print date(" g:i A | jS M Y ,",$row['datetime'])  ?></span>
    <span><?php print $date ?></span>
    <span><?php print $row['mapname'] ?></span>
    <span><?php print $row['players'] ?></span>
    </a>

</li>
<?php

}


?>
</div>

<div id=rightColumn>
        <h2>Players (<?php print $player_count; ?>)</h2>
        <table align="center" cellpadding="0" cellspacing="0" id="hopstats" class="tablesorter">
                <thead>
                	<tr>
	                        <th><?php overlib("Player Name","Name")?></th>
	                        <th><?php overlib("Players Country","Country")?></th>
	                        <th><?php overlib("Average Scores per Game + Average flag Pickups","Agressor Rating")?></th>
	                        <th><?php overlib("Average Defends(kill flag carrier) per Game + Average flag returns","Defender Rating")?></th>
	                        <th><?php overlib("Flages Defended","Flags Defended")?></th>
	                        <th><?php overlib("Highest Frags Recorded for 1 game","Frags Record")?></th>
	                        <th><?php overlib("Total Frags Ever Recorded","Total Frags")?></th>
	                        <th><?php overlib("Total Deaths","Deaths")?></th>
	                        <th><?php overlib("Accuracy %","Accuracy (%)")?></th>
	                        <th><?php overlib("Kills Per Death","Kpd")?></th>
	                        <th><?php overlib("Team Kills","TK")?></th>
	                        <th><?php overlib("Total Number of Games Played","Games")?></th>
                	</tr>
                </thead>
                <tbody>
                        <?php stats_table($sql); //Build stats table data ?>
                </tbody>
        </table>
</div>
</div>
<?php stopbench(); //Stop and display benchmark.?>

</body>
</html>
