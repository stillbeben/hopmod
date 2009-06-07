#include "scheduler.hpp"
#include "schedule_service.hpp"
#include "scripting.hpp"
#include "signals.hpp"

#include <fungu/script/lua/lua_function.hpp>

using namespace fungu;

static schedule_service scheduler;
static script::result_type sched_cubescript_code(bool,script::env::object::call_arguments &,script::env::frame *);
static inline int sched_sleep_lua_code(lua_State *);
static inline int sched_interval_lua_code(lua_State *);

void init_scheduler()
{
    static script::function<script::raw_function_type> sleep(boost::bind(sched_cubescript_code,false,_1,_2));
    get_script_env().bind_global_object(&sleep,FUNGU_OBJECT_ID("sleep"));
    
    static script::function<script::raw_function_type> interval(boost::bind(sched_cubescript_code,true,_1,_2));
    get_script_env().bind_global_object(&interval,FUNGU_OBJECT_ID("interval"));
    
    //overload the cubescript versions (unusable in lua) in the lua environment
    register_lua_function(sched_sleep_lua_code,"sleep");
    register_lua_function(sched_interval_lua_code,"interval");
    
    signal_shutdown.connect(cancel_all_scheduled,boost::signals::at_front);
}

void update_scheduler(int curtime)
{
    scheduler.update(curtime);
}

void cancel_all_scheduled()
{
    scheduler.cancel_all();
}

int run_cubescript_code(script::code_block code,script::env * e)
{
    script::env::frame frame(e);
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

script::result_type sched_cubescript_code(bool repeat,script::env::object::call_arguments & args,script::env::frame * frame)
{
    script::call_serializer cs(args,frame);
    
    int countdown = cs.deserialize(args.front(),type_tag<int>());
    args.pop_front();
    
    script::code_block code = cs.deserialize(args.front(),type_tag<script::code_block>());
    args.pop_front();
    
    scheduler.add_job(boost::bind(run_cubescript_code,code,frame->get_env()), countdown, repeat);
    
    return script::any::null_value();
}

int run_lua_code(script::env::object::shared_ptr func,script::env * e)
{
    try
    {
        script::env::frame frame(e);
        std::vector<script::any> args;
        script::arguments_container empty(args);
        return script::lexical_cast<int>(func->call(empty,&frame));
    }
    catch(script::error err)
    {
        report_script_error(new script::error_trace(err,const_string(),e->get_source_context()->clone()));
        return -1;
    }
    catch(script::error_trace * errinfo)
    {
        report_script_error(errinfo);
        return -1;
    }
}

int sched_lua_code(bool repeat,lua_State * L)
{
    int argc = lua_gettop(L);
    if(argc < 2) return luaL_error(L,"missing arguments");
    if(!lua_isnumber(L,1)) return luaL_argerror(L,1,"expecting integer type");
    int countdown = lua_tointeger(L,1);
    if(lua_type(L,2) != LUA_TFUNCTION) return luaL_argerror(L,2,"expecting function type");
    lua_pushvalue(L,2);
    
    script::env::object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted();
    
    scheduler.add_job(boost::bind(run_lua_code,luaFunctionObject,&get_script_env()), countdown, repeat);
    
    return 0;
}

int sched_sleep_lua_code(lua_State * L){return sched_lua_code(false,L);}
int sched_interval_lua_code(lua_State * L){return sched_lua_code(true,L);}

void sched_callback(int (* fun)(void *),void * closure)
{
    //FIXME memory leak for closure when job is cancelled
    scheduler.add_job(boost::bind(fun,closure),0);
}
