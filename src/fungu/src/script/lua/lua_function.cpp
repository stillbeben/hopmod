/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/lua_function.hpp"
#include "fungu/script/lua/arguments.hpp"

namespace fungu{
namespace script{
namespace lua{

lua_function::lua_function(lua_State * L,int index, const char * name)
 :m_location("")
{
    lua_getfield(L,index,name);
    assert(lua_type(L,-1) == LUA_TFUNCTION);
    set_location(L);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L)
 :m_location("")
{
    assert(lua_type(L,-1) == LUA_TFUNCTION);
    set_location(L);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::~lua_function()
{
    luaL_unref(m_lua, LUA_REGISTRYINDEX, m_ref);
}

result_type lua_function::call(call_arguments & args,env::frame * aFrame)
{
    lua_State * L = aFrame->get_env()->get_lua_state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
    int nargs = args.size();
    while(!args.empty())
    {
        //lua_pushstring(L,args.front().to_string().copy().c_str());
        args.front().push_value(L);
        args.pop_front();
    }
    if(lua_pcall(L, nargs, 1, 0) != 0)
        throw error(LUA_ERROR,boost::make_tuple(L));
    else return get_argument_value(L);
}

int lua_function::call(lua_State * L)
{
    return luaL_error(L, "calling a lua function via cubescript environment.");
}

const source_context * lua_function::get_source_context()const
{
    return &m_location;
}

void lua_function::set_location(lua_State * L)
{
    lua_pushvalue(L,-1);
    lua_Debug ar;
    ar.currentline = 0;
    ar.short_src[0] = '\0';
    if(lua_getinfo(L, ">S", &ar))
    {
        m_location = file_source_context(ar.short_src);
        m_location.set_line_number(ar.linedefined);
    }
}

} //namespace lua
} //namespace script
} //namespace fungu
