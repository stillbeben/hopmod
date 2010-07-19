/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace lua{

lua_function::lua_function(lua_State * L,int index, const char * name)
 :m_location("")
{
    lua_getfield(L,index,name);
    assert(lua_type(L, -1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L)
 :m_location("")
{
    assert(lua_type(L, -1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L, int index)
 :m_location("")
{
    assert(lua_type(L, index) == LUA_TFUNCTION);
    lua_pushvalue(L, index);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::~lua_function()
{
    luaL_unref(m_lua, LUA_REGISTRYINDEX, m_ref);
}

any lua_function::call(call_arguments & args,env_frame * aFrame)
{
    lua_State * L = aFrame->get_env()->get_lua_state();
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
    int nargs = args.size();
    while(!args.empty())
    {
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
    lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_ref);
    
    lua_Debug ar;
    ar.currentline = 0;
    ar.short_src[0] = '\0';
    if(lua_getinfo(m_lua, ">S", &ar))
    {
        m_location = file_source_context(ar.short_src);
        m_location.set_line_number(ar.linedefined);
    }
    
    return &m_location;
}

} //namespace lua
} //namespace script
} //namespace fungu
