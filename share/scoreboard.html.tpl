<html>
<head>
<title>@title scoreboard</title>
<style type="text/css">
body
{
    background-color: #000000;
    color: #f0f0f0;
    text-align:center;
    font-family: arial;
}
a{color:#FF9900;}
a:visited{font-style:italic;}
table
{
    border-collapse:collapse;
}
table td,table th{border:solid 1px  #304860;}
table th{background-color:#183048;}
table th{padding:20px;}
table td{padding:5px; color:#ffffff;}
.highlight td{background-color: #302c28;}
.footer
{
    margin-top:20px;
    color: #a0a0a0;
    font-size:small;
}
</style>
</head>
<body>
<h1>@title @(month (now)) Scoreboard</h1>
<table align="center" cellpadding="0" cellspacing="0">
<tr>@scoreboardgen_html_tblhdrs </tr>
@scoreboardgen_html_tbldata
</table>
<div class="footer">
<span id="cdate">This page was last updated @(date (now)).</span> | <a href="http://www.sauerbraten.org">Sauerbraten.org</a> | <a href="http://hopmod.e-topic.info">Hopmod</a>
</div>
</body>
</html>