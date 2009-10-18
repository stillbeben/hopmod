#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"
#include "main_io_service.hpp"
#include "free_function_scheduler.hpp"

#include <fungu/script.hpp>
#include <fungu/script/lua/lua_function.hpp>
using namespace fungu;

using namespace boost::asio;

class scheduled_function_call
{
public:
    typedef deadline_timer::duration_type duration_type;

    scheduled_function_call()
     :m_timer(get_main_io_service()),
      m_repeat(false)
    {
        
    }
    
    virtual ~scheduled_function_call()
    {
        
    }
    
    void set_wait_duration(duration_type wait_duration)
    {
        m_wait_duration = wait_duration;
    }
    
    void schedule()
    {
        m_timer.expires_from_now(m_wait_duration);
        m_timer.async_wait(boost::bind(&scheduled_function_call::timeout,this,_1));
    }
    
    void repeat(bool setRepeat)
    {
        m_repeat = setRepeat;
    }
    
    void cancel()
    {
        m_timer.cancel();
    }
private:
    virtual void call_function() = 0;
    
    void timeout(const boost::system::error_code& error)
    {
        if(error) return; //WARNING on error condition the call_function virtual method may be invalid (throws pure vm exception)
        call_function();
        if(m_repeat) schedule();
    }
    
    deadline_timer m_timer;
    duration_type m_wait_duration;
    bool m_repeat;
};

class scheduled_lua_function_call: public scheduled_function_call
{
public:
    scheduled_lua_function_call(lua_State * L, int index)
    {
        assert(lua_type(L, index) == LUA_TFUNCTION);
        lua_pushvalue(L, index);
        m_function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        m_L = L;
    }
    
    ~scheduled_lua_function_call()
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_function_ref);
    }
    
private:
    void call_function()
    {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_function_ref);
        
        if(lua_pcall(m_L, 0, 1, 0) != 0)
        {
            report_script_error(lua_tostring(m_L, -1));
            repeat(false);
            return;
        }
        
        if(lua_type(m_L, -1) != LUA_TNIL)
        {
            int new_wait_duration = lua_tointeger(m_L, -1);
            if(new_wait_duration > 0) set_wait_duration(boost::posix_time::millisec(new_wait_duration));
            else repeat(false);
        }
    }
    
    int m_function_ref;
    lua_State * m_L;
};

static free_function_scheduler free_scheduled;

static int sched_free_lua_function(lua_State * L, bool);
static inline int sched_free_lua_sleep(lua_State *);
static inline int sched_free_lua_interval(lua_State *);
static script::any sched_free_cs_function(bool, script::env_object::call_arguments &, script::env_frame *);

namespace lua{

// Lua object wrapper for scheduled_lua_function_call
class scheduled_object
{
    static const char * MT;
public:
    
    scheduled_object(lua_State * L, int index)
     :m_function(L, index)
    {
        
    }

    static int register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        lua_pushvalue(L, -1);
        
        static luaL_Reg funcs[] = {
            {"__gc", &scheduled_object::__gc},
            {"cancel", &scheduled_object::cancel},
            {"_repeat", &scheduled_object::repeat},
            {"schedule", &scheduled_object::schedule},
            {"countdown", &scheduled_object::countdown},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        
        lua_setfield(L, -1, "__index");
        
        return 0;
    }
    
    static int create(lua_State * L)
    {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        
        new (lua_newuserdata(L, sizeof(scheduled_object))) scheduled_object(L, 1);
        
        luaL_getmetatable(L, MT);
        lua_setmetatable(L, -2);
        
        return 1;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<scheduled_object *>(luaL_checkudata(L, 1, MT))->m_function.~scheduled_lua_function_call();
        return 0;
    }
    
    static int cancel(lua_State * L)
    {
        reinterpret_cast<scheduled_object *>(luaL_checkudata(L, 1, MT))->m_function.cancel();
        return 0;
    }
    
    static int repeat(lua_State * L)
    {
        luaL_checktype(L, 2, LUA_TBOOLEAN);
        reinterpret_cast<scheduled_object *>(luaL_checkudata(L, 1, MT))->m_function.repeat(lua_toboolean(L, 2));
        return 0;
    }
    
    static int schedule(lua_State * L)
    {
        reinterpret_cast<scheduled_object *>(luaL_checkudata(L, 1, MT))->m_function.schedule();
        return 0;
    }
    
    static int countdown(lua_State * L)
    {
        luaL_checkint(L, 2);
        reinterpret_cast<scheduled_object *>(luaL_checkudata(L, 1, MT))->m_function.set_wait_duration(boost::posix_time::millisec(lua_tointeger(L, 2)));
        return 0;
    }
    
    scheduled_lua_function_call m_function;
};

const char * scheduled_object::MT = "scheduled_object";

} //namespace lua

void init_scheduler()
{
    static script::function<script::raw_function_type> free_sleep(boost::bind(sched_free_cs_function, false, _1, _2));
    get_script_env().bind_global_object(&free_sleep, FUNGU_OBJECT_ID("sleep"));
    
    static script::function<script::raw_function_type> free_interval(boost::bind(sched_free_cs_function, true, _1, _2));
    get_script_env().bind_global_object(&free_interval, FUNGU_OBJECT_ID("interval"));
    
    register_lua_function(sched_free_lua_sleep, "sleep");
    register_lua_function(sched_free_lua_interval, "interval");
    
    lua::scheduled_object::register_class(get_script_env().get_lua_state());
    register_lua_function(lua::scheduled_object::create, "scheduled");
    
    signal_shutdown.connect(cancel_free_scheduled,boost::signals::at_front);
}

void cancel_free_scheduled()
{
    free_scheduled.cancel_all();
}

int call_lua_function(script::env_object::shared_ptr func, script::env * e)
{
    try
    {
        script::env_frame frame(e);
        std::vector<script::any> args;
        script::callargs empty(args);
        return script::lexical_cast<int>(func->call(empty,&frame));
    }
    catch(script::error err)
    {
        script::source_context * ctx = (e->get_source_context() ? e->get_source_context()->clone() : NULL);
        report_script_error(new script::error_trace(err,const_string(), ctx));
        return -1;
    }
    catch(script::error_trace * errinfo)
    {
        report_script_error(errinfo);
        return -1;
    }
}

int sched_free_lua_function(lua_State * L, bool repeat)
{
    int countdown = luaL_checkint(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    
    script::env_object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted();
    
    free_scheduled.schedule(boost::bind(call_lua_function, luaFunctionObject, &get_script_env()), countdown, repeat);
    
    return 0;
}

int sched_free_lua_sleep(lua_State * L)
{
    return sched_free_lua_function(L, false);
}

int sched_free_lua_interval(lua_State * L)
{
    return sched_free_lua_function(L, true);
}

int call_cs_function(script::code_block code, script::env * e)
{
    script::env_frame frame(e);
    
    try
    {
        return script::lexical_cast<int>(code.eval_each_expression(&frame));
    }
    catch(script::error err)
    {
        report_script_error(new script::error_trace(err,const_string(),code.get_source_context()->clone()));
        return -1;
    }
    catch(script::error_trace * errinfo)
    {
        report_script_error(errinfo);
        return -1;
    }
}

script::any sched_free_cs_function(bool repeat, script::env_object::call_arguments & args, script::env_frame * frame)
{
    script::callargs_serializer cs(args, frame);
    
    int countdown = cs.deserialize(args.front(),type_tag<int>());
    args.pop_front();
    
    script::code_block code = cs.deserialize(args.front(),type_tag<script::code_block>());
    args.pop_front();
    
    free_scheduled.schedule(boost::bind(call_cs_function, code, frame->get_env()), countdown, repeat);
    
    return script::any::null_value();
}

void sched_callback(int (* fun)(void *),void * closure)
{
    //FIXME memory leak for closure when job is cancelled
    free_scheduled.schedule(boost::bind(fun, closure), 0);
}

void update_scheduler(int timenow)
{
    free_scheduled.update(timenow);
}
