<html>
<head>
        <title>Chatlog</title>
</head>
<body bgcolor="grey">
<?php
include "config.php";
$max = $_GET["max"];
if (!is_numeric($max) || $max > 10000 || $max < 1)
        if ($max != null)
                die("...");
        else
                $max = 100000;
if (isset($_GET["name"])) {
        if ($_GET["name"] != null) {
                $sfn = true;
        }
}

exec("tail $path_to_log -n$max && exit", $output);
echo '<table>'."\n";
for ($i = 0; $i < count($output); $i++) {
        $m = $output[count($output)-1-$i];
    if ($m == "admin:true" || $m == "admin:false")
                continue;
        $x = explode(":", $m);
        if ($x[0] == null)
                continue;
        if ($x[1] != null && $x[1][1] != "#" && !(strlen($x[0]) > 20) && $x[0] != "new game" && strlen($x[1]) > 1) {
                $player = explode("(", $x[0]);
                $n = $player[0];
                $player = '<b><font color="orange">'.htmlspecialchars($player[0]).'</font></b>:';
                $x[1] = str_replace(" ", "&nbsp;", substr($x[1], 1, strlen($x[1])-1));
                $text = '<font color="blue">'.$x[1].'</font>';
                //$log[] = '    <tr><td>'.$player.'</td><td>'.$text.'</td></tr>'."\n";
                if ($n == $_GET["name"] || !$sfn) {
                        echo '  <tr><td>'.$player.'</td><td>'.$text.'</td></tr>'."\n";
                }
                flush();
        }
}
for ($i = 0; $i < count($log); $i++) {
        echo $log[count($log)-1-$i];
}
echo '</table>';
?>

</body>

