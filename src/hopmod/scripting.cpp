#include "scripting.hpp"
#include "scoped_setting.hpp"
#include <fungu/script/lua/object_wrapper.hpp>
#include <fungu/script/lua/lua_function.hpp>
#include <sstream>
#include <iostream>

using namespace fungu;

static script::env env;

static const char * const svrtablename = "server";
static int svrtableref = 0;
static int svrtable_index_ref = 0;

static bool binding_object_to_lua = false;
static bool binding_object_to_cubescript = false;

static void bind_object_to_lua(const_string, script::env::object *);
static inline int svrtable_index(lua_State *);
static inline int svrtable_newindex(lua_State *);
static inline void push_server_table(lua_State *);
//static inline void push_server_index_table(lua_State *,int);
static inline void register_to_server_table(lua_State *,lua_CFunction,const char *);
static std::string get_error_report(script::error_info * errinfo);

void init_scripting()
{
    script::load_corelib(env);
    
    assert(!env.get_lua_state());
    
    //startup lua
    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    env.set_lua_state(L);
    env.set_bind_observer(bind_object_to_lua);
    
    //required for lua to call cubescript functions
    lua_pushlightuserdata(L, env.get_global_scope());
    lua_setfield(L,LUA_REGISTRYINDEX, "fungu_script_global_frame");
    
    //create the server table
    lua_newtable(L);
    lua_newtable(L); // metatable
    lua_pushcclosure(L, svrtable_index, 0);
    lua_setfield(L, -2, "__index");
    lua_pushcclosure(L, svrtable_newindex, 0);
    lua_setfield(L, -2, "__newindex");
    lua_setmetatable(L, -2);
    lua_setglobal(L, svrtablename);
    
    //setup quick access to the server table
    lua_getfield(L,LUA_GLOBALSINDEX,svrtablename);
    svrtableref = luaL_ref(L,LUA_GLOBALSINDEX);
    
    //create server.index table
    push_server_table(L);
    lua_pushstring(L, "index");
    lua_newtable(L);
    lua_rawset(L, -3);
    
    //setup quick access to the server.index table
    lua_getfield(L, -1, "index");
    svrtable_index_ref = luaL_ref(L,-2);
    
    lua_pop(L,-1);
}

void shutdown_scripting()
{
    assert(env.get_lua_state());
    
    lua_State * L = env.get_lua_state();
    env.set_lua_state(NULL);
    
    push_server_table(L);
    luaL_unref(L,-1,svrtable_index_ref);
    lua_pop(L,-1);
    
    luaL_unref(L,LUA_GLOBALSINDEX,svrtableref);
    
    lua_close(L);
}

script::env & get_script_env()
{
    return env;
}

int svrtable_index(lua_State * L)
{
    const char * key = lua_tostring(L,-1);
    lua_rawgeti(L, -2, svrtable_index_ref);
    if(lua_type(L,-1)!=LUA_TTABLE) return luaL_error(L,"missing server index table");
    lua_getfield(L, -1, key);
    return 1;
}

int svrtable_newindex(lua_State * L)
{
    bool is_func = (lua_type(L,-1) == LUA_TFUNCTION);
    std::string key = lua_tostring(L, -2);
    
    //carry out the normal operation on the index table
    lua_rawgeti(L, -3, svrtable_index_ref);
    if(lua_type(L,-1)!=LUA_TTABLE) return luaL_error(L,"missing server index table");
    lua_pushvalue(L, -2); //bring value to top of stack
    lua_setfield(L, -2, key.c_str());
    //lua_pop(L,1); -- keep index table on stack
    
    //check if this call was triggered by bind_object_to_lua() being called - if so exit now to stop circling
    if(binding_object_to_lua) return 0;
    
    script::env::object * hangingObj = NULL; //object to delete if exception is thrown
    
    try
    {
        if(is_func)
        {
            scoped_setting<bool> setting(binding_object_to_cubescript,true);
            hangingObj = new script::lua::lua_function(L,-1,key.c_str());
            env.get_global_scope()->bind_global_object(hangingObj,const_string(key)).adopt_object();
            hangingObj = NULL;
        }
        else
        {
            lua_pushvalue(L,-2);
            script::env::object * obj = env.get_global_scope()->lookup_object(key);
            if(obj) obj->assign(script::lua::get_argument_value(L));
            else
            {
                scoped_setting<bool> setting(binding_object_to_cubescript,true);
                script::any_variable * newvar = new script::any_variable;
                hangingObj = newvar;
                newvar->assign(script::lua::get_argument_value(L));
                env.get_global_scope()->bind_global_object(newvar,const_string(key)).adopt_object();
                hangingObj = NULL;
            }
        }
    }
    catch(script::error_info * errinfo)
    {
        delete hangingObj;
        lua_pushstring(L,get_error_report(errinfo).c_str());
        return lua_error(L);
    }
    catch(script::error err)
    {
        delete hangingObj;
        lua_pushstring(L,err.get_error_message().c_str());
        return lua_error(L);
    }
    
    return 0;
}

void push_server_table(lua_State * L)
{
    lua_rawgeti(L, LUA_GLOBALSINDEX, svrtableref);
}

void register_to_server_table(lua_State * L,lua_CFunction func,const char * name)
{
    push_server_table(L);
    lua_pushcclosure(L, func,0);
    lua_setfield(L, -2, name);
    lua_pop(L,1);
}

void bind_object_to_lua(const_string id, script::env::object * obj)
{
    if(binding_object_to_cubescript) return;
    lua_State * L = env.get_lua_state();
    assert(!binding_object_to_lua);
    scoped_setting<bool> setting(binding_object_to_lua,true);
    push_server_table(L);
    script::lua::register_object(L, obj, id.copy().c_str());
    lua_pop(L,1);
}

std::string get_error_report(script::error_info * errinfo)
{
    const script::source_context * source = errinfo->get_root_info()->get_source_context();
    std::stringstream out;
    script::error error = errinfo->get_error();
    error.get_error_message();
    out<<"Error";
    if(source) out<<" in "<<source->get_location()<<":"<<source->get_line_number();
    out<<": "<<error.get_error_message();
    delete errinfo;
    return out.str();
}

void report_script_error(script::error_info * errinfo)
{
    std::cerr<<get_error_report(errinfo)<<std::endl;
}
