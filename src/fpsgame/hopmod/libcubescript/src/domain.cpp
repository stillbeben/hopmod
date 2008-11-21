/*   libcubescript - A CubeScript engine.  */
/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include "cubescript/domain.hpp"
#include "cubescript/symbol.hpp"
#include "cubescript/error.hpp"
#include "cubescript/exec_helpers.hpp"

namespace cubescript{

domain::domain(int flags)
 :m_parent(NULL),
  m_link(false)
{
    m_temporary = flags & TEMPORARY_DOMAIN;
}

domain::domain(domain * parent,int flags)
 :m_parent(parent),
  m_link(false)
{
    m_temporary = flags & TEMPORARY_DOMAIN;
    if(m_parent && m_parent->m_temporary) m_temporary=true;
}

domain::domain(const domain & src)
{
    m_parent=src.m_parent;
    m_temporary=src.m_temporary;
    m_link=src.m_link;
    
    for(symbol_map::const_iterator it=src.m_symbols.begin();  it!=src.m_symbols.end(); ++it)
    {
        symbol_node node=it->second;
        const char * id=it->first;
        
        register_symbol(id,node,0);
    }
}

domain::~domain()
{
    
}

domain * domain::get_parent()const
{
    return m_parent;
}

void domain::unite(const domain * src)
{
    std::copy(src->m_symbols.begin(),src->m_symbols.end(),std::inserter(m_symbols,m_symbols.begin()));
}

void domain::register_symbol(const std::string & id,symbol * s,int flags)
{
    symbol_node node;
    node.m_symbol=s;
    if(flags & ADOPT_SYMBOL) node.m_symbol_owner=boost::shared_ptr<symbol>(s);
    
    //m_id_allocs.push_back(id);
    node.m_id_alloc = &id;
    
    register_symbol(node.m_id_alloc->c_str(),node,flags);
}

void domain::register_symbol(const char * id,symbol * s,int flags)
{
    symbol_node node;
    node.m_symbol=s;
    if(flags & ADOPT_SYMBOL) node.m_symbol_owner=boost::shared_ptr<symbol>(s);
    node.m_id_alloc=NULL;
    
    register_symbol(id,node,flags);
}

void domain::register_symbol(const char * id,symbol_node & node,int flags)
{
    bool is_local_registration = !(flags & (REGISTER_ROOT | REGISTER_FIRSTNTMP));
    node.m_hide_from_descendants = is_local_registration && (flags & HIDE_FROM_DESCENDANTS);
    
    domain * registree = this;
    if(flags & REGISTER_ROOT) while(registree->m_parent) registree = registree->m_parent;
    else if(flags & REGISTER_FIRSTNTMP)
    {
        while(registree && registree->m_temporary) registree = registree->m_parent;
        if(!registree) throw error_key("runtime.domain.all_temporary_hierarchy");
    }
    
    if(node.m_id_alloc)
    {
        registree->m_id_allocs.push_back(*node.m_id_alloc);
        node.m_id_alloc = &registree->m_id_allocs.back();
    }
    
    registree->m_symbols[id] = node;
}

symbol * domain::lookup_symbol(const std::string & id,int search,int steps)const
{
    symbol_map::const_iterator it=m_symbols.find(id.c_str());
    
    if(it==m_symbols.end() || (steps > 0 && it->second.m_hide_from_descendants))
    {
        if(search & SEARCH_PARENTS && m_parent) return m_parent->lookup_symbol(id,search,steps+1);
        return NULL;
    }
    return it->second.m_symbol;
}

symbol * domain::require_symbol(const std::string & id,int search)const
{
    symbol * s=lookup_symbol(id,search);
    if(!s) throw error_key("runtime.unknown_symbol");
    return s;
}

void domain::save(std::ostream & output)const
{
    for(symbol_map::const_iterator it=m_symbols.begin(); it!=m_symbols.end(); ++it )
    {
        std::string name=it->first;
        symbol::persistable * sym=dynamic_cast<symbol::persistable *>(it->second.m_symbol);
        if(sym) output<<sym->save(name)<<std::endl;
    }
}

void domain::load(std::istream & input)
{
    exec_script(input,"",this);
}

bool domain::link(domain * newRootParent)
{
    domain * root=this;
    while(root->m_parent) root=root->m_parent;
    root->m_parent=newRootParent;
    newRootParent->m_link=true;
    return true;
}

void domain::unlink()
{
    domain * root=this;
    while(root && root->m_parent)
    {
        if(root->m_parent->m_link)
        {
            root->m_parent->m_link=false;
            root->m_parent=NULL;
        }
        root=root->m_parent;
    }
}

} //namespace cubescript
