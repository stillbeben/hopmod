#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "cube.h"
#include "hopmod.hpp"
#include "lua/modules.hpp"

#include <fungu/script/env.hpp>
#include <fungu/script/execute.hpp>
#include <fungu/script/error.hpp>
using namespace fungu;

#include <signal.h>
#include <iostream>

namespace server{
void enable_setmaster_autoapprove(bool);
void crash_handler(int signal);
void restore_server(const char * filename);
void started();
void sendservmsg(const char *);
extern string smapname;
bool selectnextgame();
}//namespace server

extern "C"{
int lua_packlibopen(lua_State *L);
} //extern "C"

static boost::signals::connection close_listenserver_slot;
static bool reload = false;

unsigned int maintenance_frequency;
static int maintenance_time = 0;
bool reloaded = false;

void init_hopmod()
{
    set_maintenance_frequency(86400000);
    
    server::enable_setmaster_autoapprove(false);
    
    struct sigaction crash_action;
    sigemptyset(&crash_action.sa_mask);
    crash_action.sa_handler = &server::crash_handler;
    crash_action.sa_flags = SA_RESETHAND;
    
    sigaction(SIGILL, &crash_action, NULL);
    sigaction(SIGABRT, &crash_action, NULL);
    sigaction(SIGFPE, &crash_action, NULL);
    sigaction(SIGBUS, &crash_action, NULL);
    sigaction(SIGSEGV, &crash_action, NULL);
    sigaction(SIGSYS, &crash_action, NULL);
    
    init_scripting();
    
    script::env & env = get_script_env();
    
    register_server_script_bindings(env);
    
    register_signals(env);
    
    close_listenserver_slot = signal_shutdown.connect(&stopgameserver);
    signal_shutdown.connect(&shutdown_scripting);
    
    init_scheduler();
    
    init_script_pipe();
    open_script_pipe("serverexec",511,get_script_env());
    
    lua_State * L = env.get_lua_state();
    
    lua::module::open_net(L);
    lua::module::open_timer(L);
    lua::module::open_crypto(L);
    lua::module::open_cubescript(L);
    lua::module::open_geoip(L);
    lua::module::open_filesystem(L);
    lua_packlibopen(L);
    
    try
    {
        fungu::script::execute_file("script/base/init.cs", get_script_env());
    }
    catch(fungu::script::error_trace * error)
    {
        report_script_error(error);
    }
}

void reload_hopmod()
{
    if(!reload)
    {
        reload = true;
        return;
    }
    else reload = false;
    
    signal_reloadhopmod();
    
    reloaded = true;
    
    close_listenserver_slot.block();
    signal_shutdown(SHUTDOWN_RELOAD);
    close_listenserver_slot.unblock();
    
    disconnect_all_slots();
    
    init_hopmod();
    server::started();
    std::cout<<"-> Reloaded Hopmod."<<std::endl;
}

void update_hopmod()
{
    if(reload) reload_hopmod();
    
    run_script_pipe_service(totalmillis);
    
    update_scheduler(totalmillis);
    cleanup_dead_slots();
    
    if(maintenance_frequency != 0 && totalmillis > maintenance_time && !hasnonlocalclients())
    {
        signal_maintenance();
        maintenance_time = totalmillis + maintenance_frequency;
    }
}

namespace server{

void started()
{
    signal_started();
    if(!server::smapname[0]) selectnextgame();
    
    if(access("log/restore", R_OK) == 0) restore_server("log/restore");
}

void shutdown()
{
    signal_shutdown(SHUTDOWN_NORMAL);
    stop_restarter();
    exit(0);
}

} //namespace server

void restart_now()
{
    server::sendservmsg("Server restarting...");
    start_restarter();
    signal_shutdown(SHUTDOWN_RESTART);
    exit(0);
}

void set_maintenance_frequency(unsigned int frequency)
{
    maintenance_frequency = frequency;
    maintenance_time = totalmillis + frequency;
}
