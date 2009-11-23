<?php

function isTeamMode($gamemode){
    
    $modes = array(
        "ffa"               => false,
        "coop-edit"         => false,
        "teamplay"          => true,
        "instagib"          => false,
        "instagib team"     => true,
        "efficiency"        => false,
        "efficiency team"   => true,
        "tactics"           => false,
        "tactics teams"     => true,
        "capture"           => true,
        "regen capture"     => true,
        "ctf"               => true,
        "insta ctf"         => true,
        "protect"           => true,
        "insta protect"     => true
    );
    
    return $modes[$gamemode];
}

function isTeamWin($gamemode, $score) {
    if($gamemode == "insta ctf" && $score == 10) return true;
    return false;
}

function abbrevGamemodeName($gamemode) {

    $modes = array(
        "ffa"               => "ffa",
        "coop-edit"         => "coop",
        "teamplay"          => "teamplay",
        "instagib"          => "insta",
        "instagib team"     => "instateam",
        "efficiency"        => "eff",
        "efficiency team"   => "effteam",
        "tactics"           => "tactics",
        "tactics teams"     => "tacticsteam",
        "capture"           => "capture",
        "regen capture"     => "regen",
        "ctf"               => "ctf",
        "insta ctf"         => "ictf",
        "protect"           => "protect",
        "insta protect"     => "iprotect"
    );
    
    return $modes[$gamemode];
}

?>
