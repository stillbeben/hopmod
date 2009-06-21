/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "fungu/script/pch.hpp"
#endif

#include "fungu/script/env.hpp"

#ifdef FUNGU_WITH_LUA
#include "fungu/script/lua/push_value.hpp"
#include "fungu/script/lua/arguments.hpp"
#endif

namespace fungu{
namespace script{

env::object::object()
:m_refcount(0),
 m_flags(0)
{
    
}

env::object::~object()
{

}

env::object::object_type env::object::get_object_type()const
{
    return env::object::UNCLASSIFIED_OBJECT;
}

#ifdef FUNGU_WITH_LUA

int env::object::call(lua_State * L)
{
    int argc = lua_gettop(L);
    
    std::vector<any> args;
    for(int i = 1; i <= argc; i++)
        args.push_back(lua::get_argument_value(L));
    
    lua_getfield(L, LUA_REGISTRYINDEX, "fungu_script_env");
    env * environment = reinterpret_cast<env *>(lua_touserdata(L, -1));
    if(!environment) return luaL_error(L, "missing 'fungu_script_env' field in lua registry");
    
    arguments_container callargs(args);
    frame callframe(environment);
    
    result_type result = call(callargs, &callframe);
    if(result.empty()) return 0;
    else
    {
        lua_pushstring(L, result.to_string().copy().c_str());
        return 1;
    }
}

void env::object::value(lua_State * L)
{
    //return lua::push_value(L, value().to_string());
    value().push_value(L);
}

#endif

result_type env::object::value()
{
    return get_shared_ptr();
}

void env::object::assign(const any &)
{
    throw error(NO_WRITE);
}

env::object * env::object::lookup_member(const_string id)
{
    return NULL;
}

env::object::member_iterator * env::object::first_member()const
{
    return NULL;
}

void env::object::add_ref()
{
    m_refcount++;
}

env::object & env::object::unref()
{
    m_refcount--;
    return *this;
}

unsigned int env::object::get_refcount()const
{
    return m_refcount;
}

env::object & env::object::set_temporary()
{
    assert(m_refcount == 0);
    m_flags |= TEMPORARY_OBJECT;
    return *this;
}

env::object & env::object::set_adopted()
{
    assert(!is_temporary());
    m_flags |= ADOPTED_OBJECT;
    return *this;
}

bool env::object::is_temporary()const
{
    return (m_flags & TEMPORARY_OBJECT);
}

bool env::object::is_adopted()const
{
    return (m_flags & ADOPTED_OBJECT);
}

env::object::shared_ptr env::object::get_shared_ptr()
{
    if(is_temporary()) throw error(UNSUPPORTED);//TODO specialised error code
    return shared_ptr(this);
}

env::object::shared_ptr env::object::get_shared_ptr(object * obj)
{
    if(obj) return obj->get_shared_ptr();
    else return NULL;
}

} //namespace script
} //namespace fungu
