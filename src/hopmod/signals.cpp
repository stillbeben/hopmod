#include "signals.hpp"
#include "scripting.hpp"
#include <fungu/script/slot_factory.hpp>
#include <fungu/script/lua/lua_function.hpp>

using namespace fungu;

static script::env env;
static script::slot_factory slots;

boost::signal<void (int)> signal_connect;
boost::signal<void (int,const char *)> signal_disconnect;
boost::signal<void (const char *,const char *)> signal_failedconnect;
boost::signal<void (int,int,std::string,std::string)> signal_kick;
boost::signal<int (int,const char *), proceed> signal_text;
boost::signal<int (int,const char *), proceed> signal_sayteam;
boost::signal<void ()> signal_shutdown;
boost::signal<void ()> signal_intermission;
boost::signal<void (int)> signal_timeupdate;
boost::signal<void (const char *,const char *)> signal_mapchange;
boost::signal<int (int,const char *,const char *), proceed> signal_mapvote;
boost::signal<void ()> signal_setnextgame;

static script::any proceed_error_handler(script::error_info * errinfo)
{
    report_script_error(errinfo);
    return true;
}

static script::any normal_error_handler(script::error_info * errinfo)
{
    report_script_error(errinfo);
    return script::any::null_value();
}

static int lua_event_handler_function(lua_State * L)
{
    int argc = lua_gettop(L);
    if(argc < 2) return luaL_error(L,"missing arguments");
    const char * name = lua_tostring(L,1);
    if(!name) return luaL_argerror(L, 1,"expecting string type for name");
    if(lua_type(L,2) != LUA_TFUNCTION) return luaL_argerror(L,2,"expecting function type for handler");
    lua_pushvalue(L,2);
    
    script::env::object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted_flag();
    
    int handle = slots.create_slot(name, luaFunctionObject, env.get_global_scope());
    lua_pushinteger(L, handle);
    return 1;
}

void register_signals(script::env & env)
{
    ::env = env;
    
    slots.register_signal(signal_connect,"connect",normal_error_handler);
    slots.register_signal(signal_disconnect,"disconnect",normal_error_handler);
    slots.register_signal(signal_failedconnect, "failedconnect",normal_error_handler);
    slots.register_signal(signal_kick,"kick",normal_error_handler);
    slots.register_signal(signal_text,"text",proceed_error_handler);
    slots.register_signal(signal_shutdown,"shutdown",normal_error_handler);
    slots.register_signal(signal_intermission,"intermission", normal_error_handler);
    slots.register_signal(signal_timeupdate,"timeupdate", normal_error_handler);
    slots.register_signal(signal_mapchange,"mapchange", normal_error_handler);
    slots.register_signal(signal_mapvote, "mapvote", proceed_error_handler);
    slots.register_signal(signal_setnextgame, "setnextgame", normal_error_handler);
    
    register_lua_function(lua_event_handler_function,"event_handler");
}
