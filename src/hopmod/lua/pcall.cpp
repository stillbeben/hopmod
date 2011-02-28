#include <lua.hpp>
#include <iostream>
#include "lua/pcall.hpp"

#define ERROR_HANDLER_KEY "pcall_error_handler"

namespace lua{

static int error_handler(lua_State * L)
{
    int argc = lua_gettop(L);
    if(!get_error_handler(L))
    {
        lua_pop(L, 1);
        return argc;
    }
    lua_insert(L, 1);
    lua_pcall(L, argc, LUA_MULTRET, 0);
    return lua_gettop(L);
}

int pcall(lua_State * L, int nargs, int nresults)
{
    lua_pushcfunction(L, error_handler);
    lua_insert(L, 1);
    int status = lua_pcall(L, nargs, nresults, 1);
    lua_remove(L, 1);
    return status;
}

void set_error_handler(lua_State * L, lua_CFunction handler_function)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_pushcfunction(L, handler_function);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void unset_error_handler(lua_State * L)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

bool get_error_handler(lua_State * L)
{
    lua_pushliteral(L, ERROR_HANDLER_KEY);
    lua_rawget(L, LUA_REGISTRYINDEX);
    return lua_type(L, -1) == LUA_TFUNCTION;
}

lua_CFunction get_pcall_error_function()
{
    return error_handler;
}

bool is_callable(lua_State * L, int index)
{
    int type = lua_type(L, index);
    if(type == LUA_TFUNCTION) return true;
    if(type == LUA_TUSERDATA)
    {
        lua_getmetatable(L, index);
        lua_getfield(L, -1, "__call");
        bool is_not_nil = lua_type(L, -1) != LUA_TNIL;
        lua_pop(L, 2);
        return is_not_nil;
    }
    return false;
}

} //namespace lua
