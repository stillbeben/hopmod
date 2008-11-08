<?php

function GetHop($cubescript) {
        $content_length = strlen($cubescript);
        $headers= "POST /serverexec HTTP/1.0\r\nContent-type: text/cubescript\r\nHost: 127.0.0.1:7894\r\nContent-length: $content_length\r\n\r\n";
        $fp = fsockopen("127.0.0.1:7894");
        if (!$fp) return false;
        fputs($fp, $headers);
        fputs($fp, $cubescript);
        $ret = "";
        while (!feof($fp)) {
                $ret = fgets($fp, 1024);
        }
        fclose($fp);
        return $ret;
}
function overlib($overtext) {
        print "<a  href=\"javascript:void(0);\" onmouseover=\"return overlib('$overtext');\" onmouseout=\"return nd();\">" ;
}
function setup_pdo_statsdb($stats_db_filename) {
	try {
	        $dbh = new PDO("sqlite:$stats_db_filename");
	}
	catch(PDOException $e)
	{
	        echo $e->getMessage();
	}
	return $dbh;
}
?>
