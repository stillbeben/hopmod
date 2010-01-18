/*
    Hopmod Web Admin Control Panel
    Copyright (C) 2010 Graham Daws
*/

var eventListeners = [];
var clients = {};
var teams = [];
var spectators = [];
var server = {};

$(document).ready(function(){
    
    hideServerStatus();
    
    createCommandShell();
    createChatShell();
    
    server.updateVars = [
        "web_admin_username", "servername", "gamemode"
    ];
    
    server.update();
    
    clients_init();
    clients_update();
    
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

function createCommandShell(){

    var commandShell = document.getElementById("command-shell");
    if(!commandShell) return;
    
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
    $(chatShell).addClass("text-shell");
    
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
    
    function addTextMessage(playerName, text){
        
        var message = document.createElement("p");
        message.className = "chat-shell-message";
        
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
    
    var eventListener = {
        eventName:"text",
        handler:function(cn, text){
            var clientInfo = clients[cn];
            if(!clientInfo) clientInfo = {name:"<unknown>"};
            var playerName = clientInfo.name + "(" + cn + ")";
            addTextMessage(playerName, text);
        }
    };
    eventListeners.push(eventListener);
    
    $(input).keypress(function(e){
    
        switch(e.which){
            case 13: // enter
            {
                if(this.value.length == 0) return;
                
                executeCommand("console [" + server.web_admin_username + "] [" + this.value + "]", function(status, responseBody){
                    addTextMessage(server.web_admin_username, input.value);
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
                $.each(reactors[event.name], function(){
                    if(this.handler) this.handler.apply(this, event.args);
                });
            });
            
            eventLoop(listenerURI);
        });
    }
}

clients_init = function(){
    eventListeners.push({eventName:"connect", handler:clients_update});
    eventListeners.push({eventName:"disconnect", handler:clients_update});
    eventListeners.push({eventName:"rename", handler:clients_update});
    eventListeners.push({eventName:"spectator", handler:clients_update});
    eventListeners.push({eventName:"frag", handler:clients_frag});
}

clients_update = function(){
    
    $.getJSON("/clients", function(response, textStatus){
        
        if(textStatus != "success"){
            updateServerStatus("server-status-error", "Clients update failure");
            return;
        }
        
        spectators = [];
        teams = {};
        
        $.each(response, function(){
            
            clients[this.cn] = this;
            
            if(this.status != "spectator"){
                
                if(this.team){
                    (teams[this.team] = teams[this.team] || []).push(this);
                }
            }
            else{
                spectators.push(this);
            }
        });
        updatePlayersDiv();
    });
}

clients_frag = function(target, actor){
    var deaths = ++clients[target].deaths;
    var frags = ++clients[actor].frags;
    $(clients[target].tableRow.deaths).text(deaths);
    $(clients[actor].tableRow.frags).text(frags);
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
        
    }, "json");
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
        {label:"Priv", key:"priv"},
        {label:"Name", key:"name"},
        {label:"Ping", key:"ping"},
        {label:"Frags", key:"frags"},
        {label:"Deaths", key:"deaths"},
        {label:"Teamkills", key:"teamkills"}
    ]);
    $.each(playersCollection, function(){
        clients[this.cn].tableRow = table.row(this);
    });
    table.attachTo(tableContainer);
    parent.appendChild(tableContainer);
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
        {label:"Name", key:"name"}
    ]);
    $.each(specsCollection, function(){
        clients[this.cn].tableRow = table.row(this);
    });
    table.attachTo(tableContainer);
    parent.appendChild(tableContainer);
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
    
    if(isTeamMode(server.gamemode)){
        $.each(teams, function(name){
            createPlayersTable(playersDiv, this, name);
        });
    }
    else{
        createPlayersTable(playersDiv, clients);
    }
    
    playersDiv.appendChild(createClearDiv());
    if(spectators.length) createSpectatorsTable(playersDiv, spectators);
}

function createClearDiv(){
    var clear = document.createElement("div");
    clear.className = "clear";
    return clear;
}
