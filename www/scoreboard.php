<?php
include("includes/geoip.inc");
include("includes/hopmod.php");


// Start session for session vars
session_start();
// Check for any http GET activity
check_get();

// Start page benchmark
startbench();

// Pull Variables from Running Hopmod Server
serverDetails();
// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($stats_db_filename);

$rows_per_page = 50;
$pager_query = "
        select COUNT(*)
        from
                (select name,
                        frags,
                        count(name) as TotalGames
                from players
                        inner join matches on players.match_id=matches.id
                        inner join ctfplayers on players.id=ctfplayers.player_id
                where matches.datetime > (strftime(\"%s\",\"now\",\"".$_SESSION['querydate']."\"))  and frags > 0 group by name)
        where TotalGames >= ". $_SESSION['MinimumGames']."
";
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
<div style="float: left"> <img src="images/hopmod.png" /> </div><br /><br /><br /><br />
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

<div style="float: right " id="pagebar">
<?php build_pager($_GET['page'],$pager_query,100); //Generate Pager Bar ?>
</div>
</div>
<?php stats_table(); //Build stats table data ?> 
</tbody>
</table>
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>
