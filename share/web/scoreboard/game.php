<?php
include("utils.inc.php");
include("sauer.inc.php");

$STATS_DSN = "sqlite:/home/sauerbraten/coe_stats.sqlite";

$db = new PDO($STATS_DSN);
if(!$db) requestFailure("Database Error: unable to make a connection");

$game_id = $_GET["id"];

$select_game_row = $db->prepare("SELECT * FROM games WHERE id = :id");
$select_team_rows = $db->prepare("SELECT * FROM teams WHERE game_id = :id ORDER BY score DESC");
$select_player_rows = $db->prepare("SELECT * FROM players WHERE game_id = :id ORDER BY score DESC, deaths ASC");

$select_game_row->bindValue(":id", $game_id);
$select_team_rows->bindValue(":id", $game_id);
$select_player_rows->bindValue(":id", $game_id);

if(!$select_game_row->execute()){
    
}

$game = $select_game_row->fetch();
if(!$game) requestFailure("Game Not Found");

if(!$select_team_rows->execute()){
    
}

$teams = array();
while($team = $select_team_rows->fetch()){
    $teams[$team["id"]] = $team;
}

if(!$select_player_rows->execute()){
    
}

$players = $select_player_rows->fetchAll();

if(isTeamMode($game["gamemode"])){
    foreach($players as $player){
        $teams[$player["team_id"]]["players"][count($teams[$player["team_id"]]["players"])] = $player;
    }
}

$displayGameInfo = array(
    0 => array("name" => "Date", "value" => strftime("%a, %b %e, %Y at %I:%M %p", $game["datetime"])),
    1 => array("name" => "Duration", "value" => $game["duration"] . " mins"),
    2 => array("name" => "Players", "value" => formatPlayerCount($game["players"],$game["bots"]))
);

$title = sprintf("Game #%d: %s %s", $game_id, abbrevGamemodeName($game["gamemode"]), $game["mapname"]); 

function formatTimeplayed($timeplayed){
    $minutes = $timeplayed / 60;
    if($minutes >= 1) return sprintf("%dm",round($minutes));
    else return sprintf("%ds", $timeplayed % 60);
}

function formatPlayerCount($humans, $bots){
    $total = $humans + $bots;
    return $total . ($bots > 0 ? " (" . $bots . " bots)" : "");
}

function formatAccuracy($hits, $shots){
    $shots = max(1, $shots);
    return sprintf("%d%%", round($hits/$shots*100));
}

function printPlayersTable($players){
    
    $header = "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n
            <thead><th>Name</th><th>Frags</th><th>Deaths</th><th>Accuracy</th><th>Timeplayed</th><th class=\"player_table_tags\"></th></thead>\n
            <tbody>";
    
    $body = "";
    foreach($players as $player){
        
        $set_name_css_class = "";
        if($player["botskill"] != 0) $set_name_css_class = "class = \"bot\"";
        
        $playerUrl = "player.php?name=" . urlencode($player["name"]); 
        
        $extraFragsInfo = sprintf("%s teamkills", $player["teamkills"]);
        $extraDeathsInfo = sprintf("%d suicides", $player["suicides"]);
        
        $accuracy = formatAccuracy($player["hits"], $player["shots"]);
        
        $timeplayed = formatTimeplayed($player["timeplayed"]);
        
        $tags = "";
        if(!$player["finished"]) $tags.= "<img src=\"images/dnf.png\" title=\"Did Not Finish\"/>";
        
        $body = $body . "<tr><td><a $set_name_css_class href=\"$playerUrl\">$player[name]</a></td><td><span title=\"$extraFragsInfo\">$player[frags]</span></td><td><span title=\"$extraDeathsInfo\">$player[deaths]</span></td><td>$accuracy</td><td>$timeplayed</td><td>$tags</td></tr>\n";
    }
    
    $footer = "</tbody></table>";
    
    return $header . $body . $footer;
}

?>
<?php include("header.inc.php");?>

<div class="box" id="gameinfo">
<?=printPropertyTable($displayGameInfo)?>
</div>
<div class="box">
    <div class="scoreboard">
        <div id="gametitle">
            <span><?=$game["gamemode"]?></span> | <span><img src="images/maps/<?=$game["mapname"]?>.jpg" style="width:64px; height:64px; vertical-align:-50%"/> <?=$game["mapname"]?></span>
        </div>
        <?php
    if(count($teams)){
        foreach($teams as $team){
            echo "<div class=\"team\">";
            
            $css_id = "";
            if($team["name"]=="good") $css_id = "team_good";
            else if($team["name"]=="evil") $css_id = "team_evil";
            
            $score = $team["score"];
            if(isTeamWin($game["gamemode"], $team["score"])) $score = "WIN";
            
            echo "<div class=\"team_name\" id=\"$css_id\">$team[name]: $score</div>";
            echo printPlayersTable($team["players"]);
            
            echo "<div style=\"clear:both\">&nbsp;</div></div>";
        }
    }
    else{
        echo "<div class=\"team\">";
        echo printPlayersTable($players);
        echo "</div>";
    }
 ?>
    </div>
</div>
<?php include("footer.inc.php");?>
