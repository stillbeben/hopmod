<?php

date_default_timezone_set("UTC");

function requestFailure($title){
    include("header.inc.php");
    echo "<div class=\"box\"><span class=\"errormsg\">$title</span></div>";
    include("footer.inc.php");
    die;
}

function printPropertyTable($props){
    $header = "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
    $body = "";
    foreach($props as $prop){
        $body = $body . "<tr><th>$prop[name]</th><td>$prop[value]</td></tr>";
    }
    $footer = "</table>";
    return $header . $body . $footer;    
}

?>