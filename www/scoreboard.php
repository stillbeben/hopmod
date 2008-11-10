<?php
include("includes/geoip.inc");
include("includes/hopmod.php");

// Start session for session vars
session_start();

// Start page benchmark
startbench();

// Check for any http GET activity
check_get();

// Pull Variables from Running Hopmod Server
$stats_db_filename = GetHop("value absolute_stats_db_filename");
if ( ! $stats_db_filename ) { $stats_db_filename = "../scripts/stats/data/stats.db"; } //Attempt a reasonable guess
$server_title = GetHop("value title");
if ( ! $server_title ) { $server_title = "HOPMOD Server";} //Set it to something

// Setup statsdb and assign it to an object.
$dbh = setup_pdo_statsdb($stats_db_filename);

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
<?php build_pager($_GET['page']); //Generate Pager Bar ?>
</div>

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
		<th><?php overlib("Total Deaths","Total Deaths")?></th>
		<th><?php overlib("Accuracy %","Accuracy (%)")?></th>
		<th><?php overlib("Kills Per Death","Kpd")?></th>
		<th><?php overlib("Team Kills","TK")?></th>
		<th><?php overlib("Total Number of Games Played","Games")?></th>
	</tr>
	</thead>
	<tbody>
<?php stats_table(); //Build stats table data ?> 
</tbody>
</table>
<div class="footer">
<span id="cdate">This page was last updated <?php print date("F j, Y, g:i a"); ?> .</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
<?php stopbench(); //Stop and display benchmark.?>
</div>
</body>
</html>
