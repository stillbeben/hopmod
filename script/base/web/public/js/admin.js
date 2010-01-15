
$(document).ready(function(){
    
    hideServerStatus();
    createCommandShell();
    
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

function executeCommand(commandLine, responseHandler){

    function success(data, textStatus){
        hideServerStatus();
        responseHandler(true, data);
    }

    function error(HttpObject, textStatus, errorThrown){
        
        if(HttpObject.status == 0){
            updateServerStatus("server-status-error", "Disconnected from Game Server!");
            responseHandler(false, "<connection broken>");
        }
        else{
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
    
    var outputContainer = document.createElement("div");
    outputContainer.id = "command-shell-output";
    
    var output = document.createElement("div");
    output.id = "command-shell-output-padding";
    outputContainer.appendChild(output);
    
    var input = document.createElement("input");
    input.type = "text";
    
    commandShell.appendChild(outputContainer);
    commandShell.appendChild(input);
    
    var history = [];
    var history_index = 0;
    
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
                    
                    history.push(input.value);
                    history_index = 0;
                    input.value = "";
                });
            
                break;
            }
            case 38: // up arrow
                var index = history.length - (++history_index);
                if(index >= 0 && index < history.length) input.value = history[index];
                else history_index--;
                break;
            case 40: // down arrow
                var index = history.length - (--history_index);
                if(index >= 0 && index < history.length) input.value = history[index];
                else history_index++;
                return false;
        }
        
    });
}
