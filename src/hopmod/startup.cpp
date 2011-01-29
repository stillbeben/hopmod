#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "cube.h"
#include "hopmod.hpp"
#include "main_io_service.hpp"

#include <fungu/script/env.hpp>
#include <fungu/script/execute.hpp>
#include <fungu/script/error.hpp>
using namespace fungu;

#include <signal.h>
#include <iostream>

#include <boost/thread.hpp>

namespace server{
void started();
void sendservmsg(const char *);
extern string smapname;
bool selectnextgame();
}//namespace server

int get_num_async_resolve_operations(); //defined in lua/net.cpp

void start_restarter();
void stop_restarter();

static boost::signals::connection close_listenserver_slot;
static bool reload = false;

unsigned int maintenance_frequency;
static int maintenance_time = 0;
bool reloaded = false;

static boost::thread::id main_thread;

/**
    Initializes everything in hopmod. This function is called at server startup and server reload.
*/
void init_hopmod()
{
    main_thread = boost::this_thread::get_id();

    close_listenserver_slot = signal_shutdown.connect(&stopgameserver);
    signal_shutdown.connect(boost::bind(&shutdown_lua));
    signal_shutdown.connect(&cleanup_info_files_on_shutdown);
        
    info_file("log/sauer_server.pid", "%i\n", getpid());
    set_maintenance_frequency(1000*60*60*24);
    
    init_scheduler();
    init_lua();
    
    static const char * INIT_SCRIPT = "script/base/new-init.lua";
    
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
    if(get_num_async_resolve_operations())
    {
        //Reload scripts delayed due to pending async resolve operations. Rescheduling reload...
        get_main_io_service().post(reload_hopmod_now);
        return;
    }

    signal_reloadhopmod();
    event_reloadhopmod(event_listeners(), boost::make_tuple());

    reloaded = true;

    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_RELOAD)));
    
    close_listenserver_slot.block();  // block close_listenserver_slot to keep clients connected
    signal_shutdown(SHUTDOWN_RELOAD);
    close_listenserver_slot.unblock();
    
    signal_started.disconnect_all_slots();
    signal_shutdown.disconnect_all_slots();
    signal_shutdown_scripting.disconnect_all_slots();
    signal_reloadhopmod.disconnect_all_slots();
    signal_maintenance.disconnect_all_slots();
    
    init_hopmod();
    server::started();
    std::cout<<"-> Reloaded Hopmod."<<std::endl;
}

void reload_hopmod()
{
    get_main_io_service().post(reload_hopmod_now);
}

void update_hopmod()
{
    if(reload) reload_hopmod();
    
    update_scheduler(totalmillis);
    
    if(maintenance_frequency != 0 && totalmillis > maintenance_time && !hasnonlocalclients())
    {
        signal_maintenance();
        event_maintenance(event_listeners(), boost::make_tuple());
        maintenance_time = totalmillis + maintenance_frequency;
    }
}

namespace server{

void started()
{
    /*
        Startup the serverexec named pipe service. The service runs by polling
        from update_hopmod().
    */
    //init_script_pipe();
    //open_script_pipe("serverexec",511,get_script_env());

    signal_started();
    event_started(event_listeners(), boost::make_tuple());
    if(!server::smapname[0]) selectnextgame();
}

void shutdown()
{
    if(boost::this_thread::get_id() != main_thread) return;
    
    stopgameserver(SHUTDOWN_NORMAL);
    
    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_NORMAL)));
    signal_shutdown(SHUTDOWN_NORMAL);
    
    stop_restarter();
    exit(0);
}

} //namespace server

void restart_now()
{
    server::sendservmsg("Server restarting...");
    start_restarter();
    event_shutdown(event_listeners(), boost::make_tuple(static_cast<int>(SHUTDOWN_RESTART)));
    signal_shutdown(SHUTDOWN_RESTART);
    exit(0);
}

void set_maintenance_frequency(unsigned int frequency)
{
    maintenance_frequency = frequency;
    maintenance_time = totalmillis + frequency;
}
