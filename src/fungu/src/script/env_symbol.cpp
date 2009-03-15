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

env::symbol::symbol()
 :m_object(NULL),
  m_flags(0)
{
    
}

env::symbol::~symbol()
{
    unbind_object();
}

env::symbol & env::symbol::bind_object(object * new_object)
{
    if(m_object)
    {
        if(!(m_flags & SYMBOL_OVERWRITE)) throw error(NO_BIND);
        unbind_object();
    }
    
    m_object = new_object;
    m_object->add_ref();
    
    return *this;
}

env::symbol & env::symbol::allow_rebind()
{
    m_flags |= SYMBOL_OVERWRITE;
    return *this;
}

env::symbol & env::symbol::adopt_object()
{
    m_flags |= SYMBOL_SHARED_OBJECT;
    m_object->set_adopted_flag();
    return *this;
}

env::symbol & env::symbol::set_dynamically_scoped()
{
    m_flags |= SYMBOL_DYNAMIC_CHAIN | SYMBOL_DYNAMIC_SCOPE;
    return *this;
}

void env::symbol::inherit_dynamic_chain(const symbol * src)
{
    m_flags &= ~SYMBOL_DYNAMIC_CHAIN; //reset
    m_flags |= (m_flags & SYMBOL_DYNAMIC_SCOPE || (src && src->m_flags & SYMBOL_DYNAMIC_CHAIN)) ? SYMBOL_DYNAMIC_CHAIN : 0;
}

env::object * env::symbol::get_object()const
{
    return m_object;
}

bool env::symbol::is_dynamically_scoped()const
{
    return m_flags & SYMBOL_DYNAMIC_SCOPE;
}

bool env::symbol::in_dynamic_scope_chain()const
{
    return m_flags & SYMBOL_DYNAMIC_CHAIN;
}

void env::symbol::unbind_object()
{
    if(m_object && m_object->unref().get_refcount() == 0 && 
        m_object->is_adopted()) delete m_object;
    m_object = NULL;
}

} //namespace script
} //namespace fungu
