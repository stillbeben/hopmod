/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

env::symbol::symbol()
 :m_local(NULL)
{
    
}

env::symbol::symbol(const symbol &)
{
    assert(false);
}

env::symbol::~symbol()
{
    assert(!m_local);
}

env::symbol_local * env::symbol::push_local_object(object * obj, const frame * frm)
{
    if(m_local && m_local->get_frame() == frm) 
    {
        m_local->set_object(obj);
        return m_local;
    }
    
    symbol_local * newLocal = new symbol_local(this, frm);
    
    newLocal->set_object(obj);
    newLocal->attach();
    
    return newLocal;
}

void env::symbol::set_global_object(object * obj)
{
    assert(obj != NULL);
    m_global = env::object::shared_ptr(obj);
}

void env::symbol::unset_global_object()
{
    m_global = env::object::shared_ptr();
}

env::object * env::symbol::lookup_object(const frame * frm)const
{
    object * obj = (m_local && frm ? m_local->lookup_object(frm) : NULL);
    if(!obj) obj = m_global.get();
    return obj;
}

env::symbol_local::symbol_local(symbol * sym, const frame * frm)
 :m_symbol(sym),
  m_frame(frm),
  m_frame_sibling(frm->get_last_bind())
{
    
}

env::symbol_local::symbol_local(const env::symbol_local &)
{
    assert(false);
}

env::symbol_local::~symbol_local()
{
    if(is_latest_attachment()) detach();
    #ifndef NDEBUG
    else //check that the local is not attached at any level
    {
        symbol_local * cur = m_symbol->m_local;
        while(cur)
        {
            assert(cur != this);
            cur = cur->m_super;
        }
    }
    #endif
    
    delete m_frame_sibling;
}

void env::symbol_local::attach()
{
    m_super = m_symbol->m_local;
    m_symbol->m_local = this;
}

void env::symbol_local::detach()
{
    assert(is_latest_attachment());
    m_symbol->m_local = m_super;
}

void env::symbol_local::set_object(object * obj)
{
    m_object = env::object::shared_ptr(obj);
}

env::object * env::symbol_local::get_object()const
{
    return m_object.get();
}

env::object * env::symbol_local::lookup_object(const frame * frm)const
{
    bool inScope = frm == m_frame || frm->get_scope_frame() == m_frame->get_scope_frame();
    if(inScope) return m_object.get();
    else return NULL;
}

const env::frame * env::symbol_local::get_frame()const
{
    return m_frame;
}

env::symbol_local * env::symbol_local::get_next_frame_sibling()const
{
    return m_frame_sibling;
}

bool env::symbol_local::is_latest_attachment()const
{
    return m_symbol->m_local == this;
}

} //namespace script
} //namespace fungu
