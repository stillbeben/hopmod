#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include <boost/bind.hpp>
#include "hopmod.hpp"
#include "lib/free_function_scheduler.hpp"

#include <iostream>

static free_function_scheduler free_scheduled;
void cancel_timer(int);
static void cancel_free_scheduled(int);

void init_scheduler()
{
    signal_shutdown.connect(cancel_free_scheduled, boost::signals::at_front);
}

void cancel_free_scheduled(int)
{
    //free_scheduled.cancel_all();
}

namespace lua{

static int call_scheduled_function(lua_State * L, int function_ref, bool repeat)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, function_ref);
    
    lua::event_environment & listeners = event_listeners();
    
    int status = 1;
    
    if(repeat)
    {
        listeners.add_listener("interval");
        
        if(event_interval(listeners, boost::make_tuple()))
            status = -1;
         
        listeners.clear_listeners(event_interval);
    }
    else
    {   
        listeners.add_listener("sleep");
        event_sleep(listeners, boost::make_tuple());
        
        listeners.clear_listeners(event_sleep);
        luaL_unref(L, LUA_REGISTRYINDEX, function_ref);
    }
    
    return status;
}

static int schedule_callback(lua_State * L, bool repeat)
{
    int countdown = luaL_checkint(L, 1);
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    int function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    int id = free_scheduled.schedule(boost::bind(&call_scheduled_function, L, function_ref, repeat), countdown, repeat);
    
    lua_pushinteger(L, id);
    return 1;
}

int sleep(lua_State * L)
{
    return schedule_callback(L, false);
}

int interval(lua_State * L)
{
    return schedule_callback(L, true);
}

} //namespace lua

void sched_callback(int (* fun)(void *),void * closure)
{
    //FIXME memory leak for closure when job is cancelled
    free_scheduled.schedule(boost::bind(fun, closure), 0);
}

void sched_callback(int (* fun)(void *), void * closure, int delay)
{
    free_scheduled.schedule(boost::bind(fun, closure), delay);
}

void update_scheduler(int timenow)
{
    free_scheduled.update(timenow);
}

void cancel_timer(int job_id)
{
    free_scheduled.cancel(job_id);
}

