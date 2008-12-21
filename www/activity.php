<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

// Start session for session vars
session_start();
$rows_per_page = 25;
// Start page benchmark
startbench();

// Check for any http GET activity
check_get();
if (! isset($_SESSION['days'])) { $_SESSION['days'] = 0;}
if ($_GET['select_day'] == "next") { $_SESSION['days'] = ($_SESSION['days'] + 1);header("location: activity.php");}
if ($_GET['select_day'] == "previous") { $_SESSION['days'] = ($_SESSION['days'] - 1);header("location: activity.php");}

$start_date = strtotime(($_SESSION['days']-1)." days");
$start_date = date("d F Y", $start_date);
$start_date = strtotime("$start_date");
$end_date = strtotime("+23 hours 59 minutes 59 seconds", $start_date); 


$day_matches = "
select matches.id as id,datetime,gamemode,mapname,duration,players
        from matches
        where matches.datetime between '$start_date' and '$end_date' and mapname != '' and gamemode != ''  and players > '1' order by datetime desc 
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
                round((0.0+(sum(scored)+sum(pickups)))/count(ctfplayers.player_id),2) as AgressorRating,
                round((0.0+(sum(defended)+sum(returns)))/count(ctfplayers.player_id),2) as DefenderRating
        from players
                inner join matches on players.match_id=matches.id
                outer left join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime between $start_date and $end_date and mapname != '' group by name order by Kpd desc)
	limit ".$_SESSION['paging'].",$rows_per_page ;

";
        $pager_query = "
select count(*)
from
        (select name
        from players
                inner join matches on players.match_id=matches.id
                outer left join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime between $start_date and $end_date and matches.mapname != '' group by name)

";
serverDetails(); //Get the server configuration and name.

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
                outer left join ctfplayers on players.id=ctfplayers.player_id
        where matches.datetime between $start_date and $end_date and mapname != '' group by name )

");

?>

<html>
<head>
	<title><?php print $server_title; ?> Daily Activity</title>
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
	<div style="width: 150px"><h2>Matches (<?php print $count_day_matches; ?>)</h2></div>
	<div id="pagebar">
		<a href="activity.php?select_day=previous">&#171; Previous day</a>
		<a href="activity.php?select_day=next">Next day &#187;</a>
	</div>
	<div id="leftColumn">
	<?php foreach ($dbh->query($day_matches) as $row){?>
		<li class="entrylist">
		<a href="match.php?id=<?php print $row['id'] ?>">
		    <span><?php print date(" g:i A | jS M Y ,",$row['datetime'])  ?></span>
		    <span><?php print $date ?></span>
		    <span><?php print $row['mapname'] ?></span>
		    <span><?php print $row['players'] ?></span>
		    <span><?php print $row['gamemode'] ?></span>
		</a>
		</li>
	<?php } ?>
	
	
	</div>
	<div id=rightColumn>
		<h2> Players (<?php print $player_count; ?>)</h2><br>
	<?php build_pager($_GET['page'],$pager_query); //Generate Pager Bar ?>
	<?php stats_table($sql); //Build stats table data ?>
	</div>
</div>
	<br /><br /><br /><br /><br />
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>
