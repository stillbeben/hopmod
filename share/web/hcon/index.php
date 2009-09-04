<?php
session_start();

$_SESSION['logged_in'] = false;
$_SESSION['first_load'] = false;

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html XMLNS="http://www.w3.org/1999/xhtml">
    <head>
        <title>
            Hopmod Console - hcon
        </title>
        <style TYPE="text/css">
body {
        margin: 0;
        }
        </style>
        <script TYPE="text/javascript">
</script>
    </head>
    <body>

        <div STYLE="width: 800px; height: 600px;" ID="terminal_container"></div><script TYPE="text/javascript" SRC="js/jquery-1.3.2.min.js">
</script><script TYPE="text/javascript" SRC="js/jquery.terminal.js">
</script><script TYPE="text/javascript">
        $('#terminal_container').terminal('rest.php', { custom_prompt : "hopmod&gt;",'submit_on_load' : true});
        </script>
    </body>
</html>

