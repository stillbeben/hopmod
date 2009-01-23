<?php

set_time_limit(0);
@ob_end_flush();
ob_implicit_flush(true);

class prompt {
  var $tty;

  function prompt() {
    if (substr(PHP_OS, 0, 3) == "WIN") {
      $this->tty = fOpen("\con", "rb");
    } else {
      if (!($this->tty = fOpen("/dev/tty", "r"))) {
        $this->tty = fOpen("php://stdin", "r");
      }
    }
  }

  function get($string, $length = 1024) {
    echo $string;
    $result = trim(fGets($this->tty, $length));
    return $result;
  }
}

while (0==0) {
  $cmdline = new prompt();
  $buffer = $cmdline->get("server>");
	
  if ( $buffer != "" ) { 
	$res = toserver($buffer);
	echo "$res\n";
  }
} 


function toserver($cubescript) {
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


?>
