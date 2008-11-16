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
if ( ! $stats_db_filename ) { $stats_db_filename = "../../scripts/stats/data/stats.db"; } //Attempt a reasonable guess
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
	<link rel="stylesheet" type="text/css" href="css/style.css" />
</head>
<body>

<noscript><div class="error">This page uses JavaScript for table column sorting and producing an enhanced tooltip display.</div></noscript>
</div>
<?php match_table($_GET['id']); //Build stats table data ?>
<?php stopbench(); //Stop and display benchmark.?>
</body>
</html>












