#include <cassert>
#include <lua.hpp>
#include <iostream>
#include "core_bindings.hpp"
#include "events.hpp"
#include "lua/modules.hpp"
#include "lua/library_extensions.hpp"

static void load_lua_modules();
static int on_error(lua_State *);
static void load_extra_os_functions(lua_State *);

static lua_State * L = NULL;
static lua::event_environment * event_environment = NULL;

void init_lua()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    
    load_extra_os_functions(L);
    
#ifndef NO_CORE_TABLE
    lua_newtable(L);
    int core_table = lua_gettop(L);
    
    bind_core_functions(L, core_table);

    lua_pushliteral(L, "vars");
    lua_newtable(L);
    int vars_table = lua_gettop(L);
    
    bind_core_constants(L, vars_table);
    bind_core_variables(L, vars_table);
    
    lua_settable(L, -3); // Add vars table to core table
    lua_setglobal(L, "core"); // Add core table to global table
#endif
    
    event_environment = new lua::event_environment(L, NULL, on_error);

#ifndef NO_EVENTS
    register_event_idents(*event_environment); // Setup and populate the event table
#endif

    load_lua_modules();
}

void shutdown_lua()
{
    if(!L) return;
    
    delete event_environment;
    lua_close(L);
    
    event_environment = NULL;
    L = NULL;
}

void load_extra_os_functions(lua_State * L)
{
    lua_getglobal(L, "os");
    
    if(lua_type(L, -1) != LUA_TTABLE)
    {
        std::cerr<<"Lua init error: the os table is not loaded"<<std::endl;
        return;
    }
    
    lua_pushliteral(L, "getcwd");
    lua_pushcfunction(L, lua::getcwd);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "mkfifo");
    lua_pushcfunction(L, lua::mkfifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "open_fifo");
    lua_pushcfunction(L, lua::open_fifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "usleep");
    lua_pushcfunction(L, lua::usleep);
    lua_settable(L, -3);
    
    lua_pop(L, 1);
}

lua_State * get_lua_state()
{
    return L;
}

static void load_lua_modules()
{
    lua::module::open_net(L);
    lua::module::open_timer(L);
    lua::module::open_crypto(L);
    lua::module::open_cubescript(L);
    lua::module::open_geoip(L);
    lua::module::open_filesystem(L);
    lua::module::open_http_server(L);
    lua_packlibopen(L);
#ifdef HAS_LSQLITE3
    luaopen_lsqlite3(L);
    lua_pop(L, 1);
#endif
}

lua::event_environment & event_listeners()
{
    static lua::event_environment unready_event_environment;
    if(!event_environment) return unready_event_environment;
    return *event_environment;
}

int on_error(lua_State * L)
{
    // This source code was copied from cubescript/lua_command_stack.cpp
    
    // Enclosing the error message in a table stops subsequent runtime error
    // functions from altering the error message argument. This is useful for
    // preserving an error message from the source nested pcall.

    if(lua_type(L, 1) != LUA_TTABLE)
    {
        lua_getglobal(L, "debug");
        if(lua_type(L, -1) != LUA_TTABLE)
        {
            lua_pop(L, 1);
            return 1;
        }
        
        lua_getfield(L, -1, "traceback");
        if(lua_type(L, -1) != LUA_TFUNCTION)
        {
            lua_pop(L, 1);
            return 1;
        }
        
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_pcall(L, 2, 1, 0);
        
        lua_newtable(L);
        lua_pushinteger(L, 1);
        lua_pushvalue(L, -3);
        lua_settable(L, -3);
        
        return 1;
    }
    
    lua_pushvalue(L, 1);
    return 1;
}

void log_event_error(const char * event_id, const char * error_message)
{
    event_listeners().log_error(event_id, error_message);
}

