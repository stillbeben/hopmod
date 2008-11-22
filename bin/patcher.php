<?php

// This script updates the data in the datetime column from the legacy timestamp to the unix epoch timestamp.

$stats_db_filename = "scripts/stats/data/stats.db";
try {
                $dbh = new PDO("sqlite:$stats_db_filename");
}
        catch(PDOException $e)
{
        echo $e->getMessage();
}
$update_required = "0";
$q = $dbh->query("select * from matches limit 5");
while ($row = $q->fetch(PDO::FETCH_OBJ)){
        if ( preg_match("/T/", $row->datetime) ) { $update_required = "1"; }
}
if ( $update_required == "0" ) { exit ;}

//Update if required
$j=0;
$q = $dbh->query("select id,datetime from matches");
while ($row = $q->fetch(PDO::FETCH_OBJ)){
        if ( preg_match("/T/", $row->datetime) ) { $j++;$datetime = new DateTime($row->datetime); $date = $datetime->format('U'); $dbh->exec("update matches set datetime = '$date' where id = '".$row->id."'"); }
}
$dbh = "";
print "WARNING bin/patcher.php Updated $j records!";
?>
