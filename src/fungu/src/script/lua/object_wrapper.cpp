/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/object_wrapper.hpp"

namespace fungu{
namespace script{
namespace lua{

namespace detail{

int object_wrapper(lua_State * L)
{
    env::object * obj = reinterpret_cast<env::object *>(lua_touserdata(L, lua_upvalueindex(1)));
    return obj->apply(L);
}

} //namespace detail

void push_object(lua_State * L, env::object * obj)
{
    lua_pushlightuserdata(L, obj);
    lua_pushcclosure(L, &detail::object_wrapper, 1);
}

void register_object(lua_State * L, int index,env::object * obj, const char * name)
{
    push_object(L,obj);
    lua_setfield(L, index, name);
}

void register_object(lua_State * L, env::object * obj, const char * name)
{
    register_object(L, -2, obj, name);
}

env::object * get_object(lua_State * L, int index, const char * name)
{
    lua_getfield(L,index,name);
    return get_object(L,-1);
}

env::object * get_object(lua_State * L,int index)
{
    lua_CFunction func = lua_tocfunction(L,index);
    if(!func || func != (const void *)&detail::object_wrapper) return NULL;
    lua_getupvalue(L,index,1);
    env::object * obj = reinterpret_cast<env::object *>(lua_touserdata(L,-1));
    lua_pop(L,1);
    return obj;
}

} //namespace lua
} //namespace script
} //namespace fungu
