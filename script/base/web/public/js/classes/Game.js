
function Game(server){

    var self = this;
    this.server = server;
    this.teams = {};
    
    var eventService = new EventDemultiplexer();
    var eventDispatcher = new EventDispatcher(this);
    
    function event_handler(name, func){
        eventService.addListener(name, func);
    }
    
    function getFullGameState(callback){
        server.getServerVariables(["gamemode", "map", "timeleft"], function(success, response){
            if(!success){
                if(callback) callback.apply(self, [false]);
                return;
            }
            $.each(response, function(name, value){
                self[name] = value;
            });
            if(callback) callback.apply(self, [true]);
        });
    }
    
    this.update = getFullGameState;
    
    event_handler("timeupdate", function(timeleft){
        self.timeleft = timeleft;
        eventDispatcher.signalEvent("timeupdate", timeleft);
    });
    
    event_handler("mapchange", function(map, gamemode){
        getFullGameState(function(){
             eventDispatcher.signalEvent("mapchange", map, gamemode);
        });
    });
    
    event_handler("timeupdate", function(mins){
        eventDispatcher.signalEvent("timeupdate", mins);
    });
    
    eventService.startListening();
    
    getFullGameState(function(success){
        if(!success){
            server.signalError();
            return;
        }
        eventDispatcher.signalEvent("ready");
    });
}

var GamemodeInfo = {

    "ffa" : {
        "teams"     : false,
        "items"     : true,
        "instagib"  : false
    },
    
    "coop edit" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "teamplay" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "instagib" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : true
    },
    
    "instagib team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },
    
    "efficiency" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "efficiency team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "tactics" : {
        "teams"     : false,
        "items"     : false,
        "instagib"  : false
    },
    
    "tactics team" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "capture" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "regen capture" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : false
    },
    
    "ctf" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta ctf" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },
    
    "protect" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta protect" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    },
    
    "hold" : {
        "teams"     : true,
        "items"     : true,
        "instagib"  : false
    },
    
    "insta hold" : {
        "teams"     : true,
        "items"     : false,
        "instagib"  : true
    }
};
