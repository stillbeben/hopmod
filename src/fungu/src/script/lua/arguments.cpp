/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/arguments.hpp"
#include "fungu/script/lua/lua_function.hpp"
#include "fungu/script/table.hpp"
#include <limits>

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

bool arguments::deserialize(value_type arg,target_tag<bool>)
{
    return lua_toboolean(m_stack, arg);
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

unsigned short arguments::deserialize(value_type arg, target_tag<unsigned short>)
{
    int n = lua_tointeger(m_stack, arg);
    if(!lua_isnumber(m_stack, arg) || n < 0 || n > std::numeric_limits<unsigned short>::max())
    {
        luaL_argerror(m_stack, arg, "expected positive short integer");
        assert(false);
        return 0;
    }
    return static_cast<unsigned short>(n);
}

const char * arguments::deserialize(value_type arg, target_tag<const char *>)
{
    const char * str = lua_tostring(m_stack, arg);
    if(!str)
    {
        if(lua_isboolean(m_stack, arg)) return (lua_toboolean(m_stack,arg) ? "1" : "0");
        luaL_argerror(m_stack, arg, "expected string");
        assert(false);
        return NULL;
    }
    return str;
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

arguments::value_type arguments::serialize(bool value)
{
    lua_pushboolean(m_stack, value);
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
            return lua_toboolean(L,-1);
        case LUA_TSTRING:
            return const_string(lua_tostring(L,-1));
        case LUA_TFUNCTION:
        {
            env::object * obj = new lua_function(L);
            obj->set_adopted_flag();
            return obj->get_shared_ptr();
        }
        case LUA_TTABLE:
        {
            table * outT = new table();

            lua_pushnil(L);
            while(lua_next(L, -2) != 0)
            {
                try
                {
                    any value = get_argument_value(L);
                    lua_pop(L,1);
                    std::string name = get_argument_value(L).to_string().copy();
                    outT->assign(name, value);
                }
                catch(error)
                {
                    delete outT;
                    throw;
                }
            }
            
            outT->set_adopted_flag();
            return outT->get_shared_ptr();
        }
        default:
            throw error(NO_CAST);
    }
}

} //namespace lua
} //namespace script
} //namespace fungu
