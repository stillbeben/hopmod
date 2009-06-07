/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_PUSH_VALUE_HPP
#define FUNGU_SCRIPT_LUA_PUSH_VALUE_HPP

#include "../lexical_cast.hpp"
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace fungu{
namespace script{
namespace lua{

template<typename T>
void push_value(lua_State * L, const T & value)
{
    const_string value_str = lexical_cast<const_string>(value);
    lua_pushlstring(L, value_str.begin(), value_str.length());
}

inline void push_value(lua_State * L, int value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, unsigned int value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, long value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, unsigned long value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, short value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, unsigned short value){lua_pushinteger(L, static_cast<lua_Integer>(value));}
inline void push_value(lua_State * L, bool value){lua_pushboolean(L, static_cast<int>(value));}
inline void push_value(lua_State * L, double value){lua_pushnumber(L, static_cast<lua_Number>(value));}
inline void push_value(lua_State * L, float value){lua_pushnumber(L, static_cast<lua_Number>(value));}
inline void push_value(lua_State * L, char value){lua_pushlstring(L, &value, 1);}
inline void push_value(lua_State * L, unsigned char value){lua_pushinteger(L, static_cast<lua_Integer>(value));}

inline void push_value(lua_State * L, const std::string & value){lua_pushstring(L, value.c_str());}
inline void push_value(lua_State * L, const const_string & value){lua_pushlstring(L, value.begin(), value.length());}
inline void push_value(lua_State * L, const char * value){lua_pushstring(L, value);}

} //namespace lua
} //namespace script
} //namespace fungu

#endif
