/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/arguments.hpp"

namespace fungu{
namespace script{
namespace lua{

arguments::arguments(lua_State * stack)
 :m_stack(stack),
  m_arg_index(1)
{
    
}

arguments::value_type & arguments::front()
{
    return m_arg_index;
}

void arguments::pop_front()
{
    m_arg_index++;
}

std::size_t arguments::size()const
{
    return lua_gettop(m_stack);
}

int arguments::deserialize(value_type arg,target_tag<int>)
{
    if(!lua_isnumber(m_stack, arg))
    {
        luaL_argerror(m_stack, arg, "expected integer");
        assert(false);
        return 0;
    }
    return lua_tointeger(m_stack, arg);
}

unsigned int arguments::deserialize(value_type arg, target_tag<unsigned int>)
{
    int n = lua_tointeger(m_stack, arg);
    if(!lua_isnumber(m_stack, arg) || n < 0)
    {
        luaL_argerror(m_stack, arg, "expected positive integer");
        assert(false);
        return 0;
    }
    return n;
}

const char * arguments::deserialize(value_type arg, target_tag<const char *>)
{
    if(!lua_isstring(m_stack, arg))
    {
        luaL_argerror(m_stack, arg, "expected string");
        assert(false);
        return NULL;
    }
    return lua_tostring(m_stack, arg);
}

std::string arguments::deserialize(value_type arg, target_tag<std::string>)
{
    return std::string(deserialize(arg,target_tag<const char *>()));
}

arguments::value_type arguments::serialize(const char * str)
{
    lua_pushstring(m_stack,str);
    return 1;
}

arguments::value_type arguments::serialize(const std::string & str)
{
    return serialize(str.c_str());
}

arguments::value_type arguments::serialize(int n)
{
    lua_pushinteger(m_stack, n);
    return 1;
}

arguments::value_type arguments::get_void_value()
{
    return 0;
}

any get_argument_value(lua_State * L)
{
    switch(lua_type(L,-1))
    {
        case LUA_TNIL:
            return any::null_value();
        case LUA_TNUMBER:
        {
            lua_Integer i = lua_tointeger(L,-1);
            lua_Number n = lua_tonumber(L,-1);
            if(n==i) return any(static_cast<int>(i));
            else return any(n);
        }
        case LUA_TBOOLEAN:
            return any(lua_toboolean(L,-1));
        case LUA_TSTRING:
            return any(const_string(lua_tostring(L,-1)));
        case LUA_TTABLE:    //TODO
        case LUA_TFUNCTION: //TODO self-referenced lua_function object
        default:
            throw error(NO_CAST);
    }
}

} //namespace lua
} //namespace script
} //namespace fungu
