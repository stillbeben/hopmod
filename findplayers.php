<?php
include "config.php";
if (isset($_GET["country"])) {
	if ($_GET["country"] != null) {
		$country = $_GET["country"];
		$c = true;
	}
}
if (!$c && isset($_GET["ip"])) {
}

exec("tail $path_to_log -n100000 && exit", $output);
for ($i = 0; $i < count($output); $i++) {
	if (contains($output[$i], "connected") && !contains($output[$i], "disconnected")) {
		if (contains($output[$i], $country)) {
			$x = explode("(", $output[$i]);
			if ($x[0] != null && !contains($players, $x[0])) {
				echo htmlspecialchars($x[0]).'<br>';
				flush();
			}
			$players .= $x[0].',';
		}
	}
	else
		continue;
}

function contains($var, $text) {
	if (strlen(str_replace($text, null, $var)) < strlen($var)) 
		return true;
	else 	
		return false;
}
?>
