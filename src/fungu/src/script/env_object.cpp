/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"

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
int env::object::apply(lua_State * L)
{
    return luaL_error(L,"inaccessible from lua");
}
#endif

result_type env::object::value()
{
    return get_shared_ptr();
}

void env::object::assign(const any &)
{
    throw error(UNSUPPORTED);
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

const source_context * env::object::get_source_context()const
{
    return NULL;
}

env::object & env::object::set_temporary_flag()
{
    assert(m_refcount == 0);
    m_flags |= FLAG_TEMPORARY;
    return *this;
}

env::object & env::object::set_adopted_flag()
{
    assert(!is_temporary());
    m_flags |= FLAG_ADOPTED;
    return *this;
}

bool env::object::is_temporary()const
{
    return (m_flags & FLAG_TEMPORARY);
}

bool env::object::is_adopted()const
{
    return (m_flags & FLAG_ADOPTED);
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
