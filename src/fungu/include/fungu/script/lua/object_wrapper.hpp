/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_OBJECT_WRAPPER_HPP
#define FUNGU_SCRIPT_LUA_OBJECT_WRAPPER_HPP

#include "../env.hpp"
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace fungu{
namespace script{
namespace lua{

void register_object(lua_State * L, int index,env::object * obj, const char * name);
void register_object(lua_State * L, env::object * obj, const char * name);

env::object * get_object(lua_State *,int index,const char * name);

} //namespace lua
} //namespace script
} //namespace fungu

#endif
