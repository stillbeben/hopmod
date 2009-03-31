/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/lua_function.hpp"

namespace fungu{
namespace script{
namespace lua{

lua_function::lua_function(lua_State * L,int index, const char * name)
{
    lua_getfield(L,index,name);
    assert(lua_type(L,-1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L)
{
    assert(lua_type(L,-1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::~lua_function()
{
    luaL_unref(m_lua, LUA_REGISTRYINDEX, m_ref);
}

result_type lua_function::apply(apply_arguments & args,env::frame * aFrame)
{
    lua_State * L = aFrame->get_env()->get_lua_state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
    int nargs = args.size();
    while(!args.empty())
    {
        lua_pushstring(L,args.front().to_string().copy().c_str());
        args.pop_front();
    }
    if(lua_pcall(L, nargs, 1, 0) != 0)
        throw error(LUA_ERROR,boost::make_tuple(L));
    else
    {
        if(lua_type(L, -1) == LUA_TNIL) return any::null_value();
        else return const_string(lua_tostring(L,-1)); //const_string ctor will copy the string
    }
}

int lua_function::apply(lua_State * L)
{
    return luaL_error(L, "calling a lua function via cubescript environment.");
}

} //namespace lua
} //namespace script
} //namespace fungu
