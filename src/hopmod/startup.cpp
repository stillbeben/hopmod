#include "cube.h"
#include "hopmod.hpp"
#include "extapi.hpp"
#include "main_io_service.hpp"
#include <signal.h>
#include <iostream>
#include <boost/thread.hpp>

bool reloaded = false;

static boost::thread::id main_thread;

/**
    Initializes everything in hopmod. This function is called at server startup and server reload.
*/
void init_hopmod()
{
    main_thread = boost::this_thread::get_id();

    signal_shutdown.connect(boost::bind(&shutdown_lua));
    signal_shutdown.connect(&cleanup_info_files_on_shutdown);
        
    info_file("log/sauer_server.pid", "%i\n", getpid());
    
    init_scheduler();
    init_lua();
    
    static const char * INIT_SCRIPT = "script/base/init.lua";
    
    lua_State * L = get_lua_state();
    if(luaL_loadfile(L, INIT_SCRIPT) == 0)
    {
        event_listeners().add_listener("init"); // Take the value of the top of the stack to add
        // to the init listeners table
    }
    else
    {
        std::cerr<<"error during initialization: "<<lua_tostring(L, -1)<<std::endl;
        lua_pop(L, 1);
    }
    
    event_init(event_listeners(), boost::make_tuple());
}

static void reload_hopmod_now()
{
    signal_reloadhopmod();
    event_reloadhopmod(event_listeners(), boost::make_tuple());

    reloaded = true;

    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_RELOAD)));
    
    update_scheduler(totalmillis); // Call zero wait handlers
    
    signal_shutdown(SHUTDOWN_RELOAD);
    
    signal_shutdown.disconnect_all_slots();
    signal_shutdown_scripting.disconnect_all_slots();
    signal_reloadhopmod.disconnect_all_slots();

    init_hopmod();
    server::started();
    std::cout<<"-> Reloaded Hopmod."<<std::endl;
    
    reloaded = false;
}

void reload_hopmod()
{
    get_main_io_service().post(reload_hopmod_now);
}

void update_hopmod()
{
    update_scheduler(totalmillis);
}

namespace server{

void started()
{
    event_started(event_listeners(), boost::make_tuple());
    if(!server::smapname[0]) selectnextgame();
}

static void initiate_shutdown()
{
    stopgameserver(SHUTDOWN_NORMAL);
    
    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_NORMAL)));
    signal_shutdown(SHUTDOWN_NORMAL);
    
    stop_restarter();
    
    // Now wait for the main event loop to process work that is remaining and then exit
}

void shutdown()
{
    if(boost::this_thread::get_id() != main_thread) return;
    get_main_io_service().post(initiate_shutdown);
}

} //namespace server

void restart_now()
{
    server::sendservmsg("Server restarting...");
    start_restarter();
    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_RESTART)));
    signal_shutdown(SHUTDOWN_RESTART);
}

