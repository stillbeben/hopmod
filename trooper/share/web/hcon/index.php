<?php
session_start();

$_SESSION['logged_in'] = false;
$_SESSION['first_load'] = false;

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>
      Hopmod Console - hcon
    </title>
    <script type="text/javascript" src="js/jquery-1.3.2.min.js">
</script>
    <script type="text/javascript" src="js/jquery.terminal.js">
</script>
    <style type="text/css">
/*<![CDATA[*/
        body {
        margin: 0;
        }
    /*]]>*/
    </style>
  </head>

  <body>
    <div style="margin:0 auto;width:600px; padding-top: 10px;">
      <div style="width: 600px; height: 600px;" id="terminal_container"></div>
    </div><script type="text/javascript">
//<![CDATA[
         $('#terminal_container').terminal('rest.php', { custom_prompt : "hopmod&gt;",'submit_on_load' : true});
    //]]>
    </script>
  </body>
</html>

