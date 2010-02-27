/*
    Hopmod Web Admin Control Panel
    Copyright (C) 2010 Graham Daws
*/
var appTitle;
var eventListeners = [];
var clients = {};
var teams = [];
var singles = [];
var spectators = [];
var server = {};
var ui = {};

$(document).ready(function(){
    
    appTitle = document.title;
    
    hideServerStatus();
    
    createCommandShell();
    createChatShell();
    
    server.updateVars = [
        "web_admin_session_username", "servername", "gamemode", "map", "timeleft"
    ];
    
    server.update();
    
    clients_init();
    clients_update();
    
    server_init();
    
    createListenerResource(eventListeners);
    
    $("#admin-startup").css("display", "none");
    $("#admin-container").css("display","block");
});

function updateServerStatus(className, msg){
    var serverStatus = $("#server-status");
    if(!serverStatus.size()) return;
    serverStatus.get(0).className = className;
    serverStatus.text(msg);
    serverStatus.show();
}

function hideServerStatus(){
    $("#server-status").hide();
}

function gotoLogin(){
    alert("You are not logged in");
    document.location = "/login?return=" + document.location;
}

function executeCommand(commandLine, responseHandler){
    
    function success(data, textStatus){
        hideServerStatus();
        responseHandler(true, data);
    }

    function error(HttpObject, textStatus, errorThrown){
        
        switch(HttpObject.status){
            case 0:
            case 12029:
                updateServerStatus("server-status-error", "Disconnected from Game Server!");
                responseHandler(false, "<connection broken>");
                break;
            case 401:
                gotoLogin();
                break;
            default:
                hideServerStatus();
                responseHandler(false, HttpObject.responseText);
        }
    }

    $.ajax({
        type:"POST",
        url:"/serverexec",
        contentType: "text/x-cubescript",
        data: commandLine,
        success: success,
        error: error
    });
}

server.event_handler = function(name, handler){
    eventListeners.push({eventName: name, handler: handler});
}

function createCommandShell(){

    var commandShell = document.getElementById("command-shell");
    if(!commandShell) return;
    $(commandShell).empty();
    
    var heading = document.createElement("div");
    heading.id = "command-shell-heading";
    heading.appendChild(document.createTextNode("Command Shell"));
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "command-shell-output";
    
    var output = document.createElement("div");
    output.id = "command-shell-output-padding";
    outputContainer.appendChild(output);
    
    var input = document.createElement("input");
    input.type = "text";
    
    commandShell.appendChild(heading);
    commandShell.appendChild(outputContainer);
    commandShell.appendChild(input);
    
    $(input).keypress(function(e){
    
        switch(e.which){
            case 13: // enter
            {
                if(this.value.length == 0) return;
            
                executeCommand(this.value, function(status, responseBody){
                    
                    var echo = document.createElement("div");
                    echo.className = "echo-output";
                    echo.appendChild(document.createTextNode(input.value));
                    output.appendChild(echo);
                    
                    var serverResponse = document.createElement("div");
                    
                    if(status){
                        serverResponse.className = "normal-output";
                        if(!responseBody.length) serverResponse.className = "normal-no-output";
                    }
                    else{
                        serverResponse.className = "error-output";
                    }
                    
                    serverResponse.appendChild(document.createTextNode(responseBody));
                    
                    output.appendChild(serverResponse);
                    
                    $(outputContainer).scrollTop(output.scrollHeight);
                    
                    input.value = "";
                });
            
                break;
            }
        }
        
    });
}

function createChatShell(){

    var chatShell = document.getElementById("chat-shell");
    if(!chatShell) return;
    $(chatShell).empty().addClass("text-shell");
    
    var heading = document.createElement("div");
    heading.id = "chat-shell-heading";
    heading.className="text-shell-heading";
    heading.appendChild(document.createTextNode("Chat"));
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "chat-shell-output";
    outputContainer.className="text-shell-output";
    
    var output = document.createElement("div");
    output.id = "chat-shell-output-padding";
    output.className = "text-shell-output-padding";
    outputContainer.appendChild(output);
    
    var input = document.createElement("input");
    input.type = "text";
    
    chatShell.appendChild(heading);
    chatShell.appendChild(outputContainer);
    chatShell.appendChild(input);
    
    function addTextMessage(playerName, text, className){
        
        var message = document.createElement("p");
        message.className = "chat-shell-message";
        
        if(className){
            message.className += " " + className;
        }
        
        var name = document.createElement("span");
        name.className = "chat-shell-name";
        name.appendChild(document.createTextNode(playerName));
        
        var messageText = document.createElement("span");
        messageText.className = "chat-shell-text";
        messageText.appendChild(document.createTextNode(text));
        
        var clear = document.createElement("div");
        clear.className = "clear";
        
        message.appendChild(name);
        message.appendChild(messageText);
        
        output.appendChild(message);
        output.appendChild(clear);
        
        $(outputContainer).scrollTop(output.scrollHeight);
    }
    
    server.event_handler("text", function(cn, text){
        var clientInfo = clients[cn];
        if(!clientInfo) clientInfo = {name:"<unknown>"};
        var playerName = clientInfo.name + "(" + cn + ")";
        addTextMessage(playerName, text);
    });
    
    server.event_handler("admin-message", function(admin, text){
        addTextMessage(admin, text, "admin");
    });
    
    $(input).keypress(function(e){
    
        switch(e.which){
            case 13: // enter
            {
                if(this.value.length == 0) return;
                
                executeCommand("console [" + server.web_admin_session_username + "] [" + this.value + "]", function(status, responseBody){
                    //addTextMessage(server.web_admin_session_username, input.value);
                    input.value = "";
                });
                
                break;
            }
        }
        
    });
}

function createListenerResource(reactors){
    
    var reactorMap = {};
    $.each(reactors, function(){
        var a = reactorMap[this.eventName];
        if(a === undefined) a = [];
        a.push(this);
        reactorMap[this.eventName] = a;
    });
    reactors = reactorMap;
    
    var eventsList = "[";
    var count = 0;
    $.each(reactors, function(eventName){
        if(count++ > 0) eventsList += ", ";
        eventsList += "\"" + eventName + "\"";
    });
    eventsList += "]";
    
    $.post("/listener", eventsList, function(response, textStatus){
        if(textStatus != "success"){
            updateServerStatus("server-status-error", "Failed to start event listening");
            return;
        }
        
        eventLoop(response.listenerURI);
    }, "json");

    function eventLoop(listenerURI){
        
        $.getJSON(listenerURI, function(events, textStatus){
            
            if(textStatus != "success"){
                updateServerStatus("server-status-error", "Event listening failure");
                return;
            }
            
            $.each(events, function(){
                var event = this;
                var reactor = reactors[event.name];
                if(reactor){
                    $.each(reactor, function(){
                        if(this.handler) this.handler.apply(this, event.args);
                    });
                }
            });
            
            eventLoop(listenerURI);
        });
    }
}

function addClientToPlayerTable(client){
    if(client.status != "spectator"){
        if(isTeamMode(server.gamemode)) (teams[client.team] = teams[client.team] || []).push(client);
        else singles.push(client);
    }
    else{
        spectators.push(client);
    }
}

function addClientToUiTable(client){
    var table;
    if(client.status == "spectator"){
        if(ui.spectators) table = ui.spectators;
    }
    else{
        if(isTeamMode(server.gamemode)) table = ui.teams[client.team];
        else table = ui.players;
    }
    if(!table){
        updatePlayersDiv();
        return;
    }
    clients[client.cn].tableRow = table.row(client);
    updateClientUiTableRow(clients[client.cn]);
}

function updateClientUiTableRow(client){
    $(client.tableRow.tableRowElement)
        .removeClass("no-priv")
        .removeClass("master-priv")
        .removeClass("admin-priv")
        .addClass(getPrivilegeClassName(client.priv));
}

function clients_init(){

    function onDisconnect(cn){
        clients[cn].tableRow.remove(); 
        clients[cn] = null;
    }
    
    function onPlayerFrag(target, actor){
        
        var deaths = ++clients[target].deaths;
        var frags = ++clients[actor].frags;
        
        var targetClient = clients[target];
        var actorClient = clients[actor];
        
        targetClient.tableRow.update({deaths: deaths});
        actorClient.tableRow.update({frags: frags});
        
        $(targetClient.tableRow.tableRowElement).addClass("dead");
    }
    
    function onPlayerSpawn(cn){
        if(clients[cn]) $(clients[cn].tableRow.tableRowElement).removeClass("dead");
    }
    
    function onSpectator(cn, joined){
        clients[cn].status = (joined ? "spectator" : "dead");
        clients[cn].tableRow.remove();
        resyncClientTables();
        addClientToUiTable(clients[cn]);
    }
    
    server.event_handler("connect", getSingleClientUpdate);
    server.event_handler("disconnect", onDisconnect);
    server.event_handler("maploaded", getSingleClientUpdate);
    server.event_handler("rename", getSingleClientUpdate);
    server.event_handler("spectator", onSpectator);
    server.event_handler("mapchange", clients_update);
    server.event_handler("frag", onPlayerFrag);
    server.event_handler("spawn", onPlayerSpawn);
    server.event_handler("privilege", getSingleClientUpdate);
    server.event_handler("addbot", function(ownerCn, skill, botCn){getSingleClientUpdate(botCn);});
    server.event_handler("botleft", onDisconnect);
}

function server_init(){

    server.event_handler("timeupdate", function(timeleft){
        server.timeleft = timeleft;
        updateGameInfoDiv();
    });

    server.event_handler("mapchange", function(){server.update();});
}

function getSingleClientUpdate(cn){
    $.getJSON("/clients/" + cn, function(response, textStatus){
        if(textStatus != "success"){
            updateServerStatus("server-status-error", "Clients update failure");
            return;
        }
        cn = response.cn;
        if(clients[cn]){
            for(var key in response) clients[cn][key] = response[key];
            clients[cn].tableRow.update(clients[cn]);
            if(clients[cn].status != "dead") $(clients[cn].tableRow.tableRowElement).removeClass("dead");
            updateClientUiTableRow(clients[cn]);
        }
        else{
            clients[cn] = response;
            addClientToPlayerTable(response);
            addClientToUiTable(response);
        }
    });
}

function resyncClientTables(){
    spectators = [];
    teams = {};
    singles = [];
    for(var cn in clients) addClientToPlayerTable(clients[cn]);
}

function clients_update(){
    
    $.getJSON("/clients", function(response, textStatus){
        
        if(textStatus != "success"){
            updateServerStatus("server-status-error", "Clients update failure");
            return;
        }
        
        spectators = [];
        teams = {};
        singles = [];
        
        $.each(response, function(){
            clients[this.cn] = this;
            addClientToPlayerTable(this);
        });
        
        updatePlayersDiv();
    });
}

function swapTableRowElements(firstRow, secondRow){
    secondRow.parentNode.insertBefore(secondRow, firstRow);
    firstRow.parentNode.removeChild(secondRow);
}

server.update = function(){
    
    var queryvars = $.toJSON(this.updateVars);
    
    $.post("/queryvars", queryvars, function(response, textStatus){
        
        if(textStatus != "success"){
            updateServerStatus("server-status-error", "Server update failure");
            return;
        }
        
        $.each(response, function(name, value){
            server[name] = value;
        });
        
        updateServerInfoDiv();
        updateGameInfoDiv();
        
    }, "json");
}

function createPlayerControlLinks(data){
    var kick = document.createElement("a");
    kick.className = "kick-button";
    kick.href="#";
    kick.title="Kick";
    kick.onclick = function(){
        var yes = confirm("You are you sure you want to kick " + data.name + "(" + data.cn + ")");
        if(yes){
            executeCommand("kick " + data.cn);
        }
        return false;
    }
    return kick;
}

function createPlayersTable(parent, playersCollection, team){
    var tableContainer = document.createElement("div");
    tableContainer.className = "team";
    tableContainer.id = "team-" + team;
    var heading = document.createElement("h2");
    heading.appendChild(document.createTextNode(team || "Players"));
    tableContainer.appendChild(heading);
    
    var table = new HtmlTable();
    table.columns([
        {label:"CN", key:"cn"},
        {label:"IP Addr", key:"ip"},
        {label:"Name", key:"name"},
        {label:"Ping", key:"ping"},
        {label:"Frags", key:"frags"},
        {label:"Deaths", key:"deaths"},
        {label:"Teamkills", key:"teamkills"},
        {label:"", cellFunction: createPlayerControlLinks, className:"player-control-links"}
    ], [{key:"frags", order: descendingOrder}, {key:"deaths", order: ascendingOrder}]);
    
    $.each(playersCollection, function(){
        clients[this.cn].tableRow = table.row(this, getPrivilegeClassName(this.priv));
        if(this.status == "dead") $(clients[this.cn].tableRow.tableRowElement).addClass("dead");
    });
    
    table.attachTo(tableContainer);
    parent.appendChild(tableContainer);
    return table;
}

function createSpectatorsTable(parent, specsCollection){
    var tableContainer = document.createElement("div");
    tableContainer.className = "spectators";
    var heading = document.createElement("h2");
    heading.appendChild(document.createTextNode("Spectators"));
    tableContainer.appendChild(heading);
    var table = new HtmlTable();
    table.columns([
        {label:"CN", key:"cn"},
        {label:"IP Addr", key:"ip"},
        {label:"Priv", key:"priv"},
        {label:"Name", key:"name"},
        {label:"", cellFunction: createPlayerControlLinks, className:"player-control-links"}
    ]);
    $.each(specsCollection, function(){
        clients[this.cn].tableRow = table.row(this);
    });
    table.attachTo(tableContainer);
    parent.appendChild(tableContainer);
    return table;
}

function isTeamMode(gamemode){
    switch(gamemode){
        case "insta ctf":
            return true;
    }
    return false;
}

function updatePlayersDiv(){

    var playersDiv = document.getElementById("players");
    if(!playersDiv) return;
    $(playersDiv).empty();
    
    ui.spectators = null;
    ui.teams = {};
    ui.players = null;
    
    if(isTeamMode(server.gamemode)){
        $.each(teams, function(name){
            ui.team[name] = createPlayersTable(playersDiv, this, name);
        });
    }
    else{
        if(singles.length) ui.players = createPlayersTable(playersDiv, singles);
    }
    
    playersDiv.appendChild(createClearDiv());
    if(spectators.length){
        ui.spectators = createSpectatorsTable(playersDiv, spectators);
    }
}

function updateServerInfoDiv(){
    var infoDiv = document.getElementById("server-info");
    if(!infoDiv) return;
    $(infoDiv).empty();
    
    var infoList = document.createElement("ul");
    infoList.className = "";
    
    function createListItem(text, id){
        var li = document.createElement("li");
        li.appendChild(document.createTextNode(text));
        li.id = id;
        infoList.appendChild(li);
    }
    
    document.title = server.servername + " - " + appTitle;
    createListItem(server.servername, "server-name");
    
    createListItem(appTitle, "app-title");
    
    infoDiv.appendChild(infoList);
}

function updateGameInfoDiv(){
    var infoDiv = document.getElementById("game-info");
    if(!infoDiv) return;
    $(infoDiv).empty();
    
    var gamemode = document.createElement("span");
    var mapname = document.createElement("span");
    var minsleft = document.createElement("span");
    
    gamemode.id = "gamemode";
    gamemode.appendChild(document.createTextNode(server.gamemode));
    
    mapname.id = "mapname";
    mapname.appendChild(document.createTextNode(server.map));
    
    minsleft.id = "timeleft";
    var timeleft_message = "";
    if(server.timeleft > 0) timeleftMessage = server.timeleft + " mins left";
    else timeleftMessage = "intermission";
    minsleft.appendChild(document.createTextNode(timeleftMessage));
    
    infoDiv.appendChild(gamemode);
    infoDiv.appendChild(document.createTextNode(" - "));
    infoDiv.appendChild(mapname);
    infoDiv.appendChild(document.createTextNode(" - "));
    infoDiv.appendChild(minsleft);
}

function createClearDiv(){
    var clear = document.createElement("div");
    clear.className = "clear";
    return clear;
}

function getPrivilegeClassName(privName){
    var privClassName = "no-priv";
    if(privName == "master"){
        privClassName = "master-priv";
    }
    else if(privName == "admin"){
        privClassName = "admin-priv";
    }
    return privClassName;
}
