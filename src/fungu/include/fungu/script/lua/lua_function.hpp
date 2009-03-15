/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_LUA_FUNCTION_HPP
#define FUNGU_SCRIPT_LUA_LUA_FUNCTION_HPP

#include "../env.hpp"
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace fungu{
namespace script{
namespace lua{

class lua_function:public env::object
{
public:
    lua_function(lua_State *,int index, const char * name);
    result_type apply(apply_arguments & args,env::frame * aFrame);
    int apply(lua_State * L);
private:
    int m_ref;
};

} //namespace lua
} //namespace script
} //namespace fungu

#endif
