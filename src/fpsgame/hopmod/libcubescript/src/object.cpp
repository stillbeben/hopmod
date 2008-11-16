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

#include <boost/bind.hpp>
#include "cubescript/object.hpp"
#include "cubescript/error.hpp"

namespace cubescript{


proto_object::proto_object()
 :m_func_value(boost::bind(&proto_object::get_member_value,this,_1)),
  m_func_super(boost::bind(&proto_object::call_super_operation,this,_1,_2)),
  m_func_function(boost::bind(&proto_object::create_function,this,_1,_2))
{
    register_members();
}

proto_object::proto_object(proto_object * superclass)
 :m_func_value(boost::bind(&proto_object::get_member_value,this,_1)),
  m_func_super(boost::bind(&proto_object::call_super_operation,this,_1,_2)),
  m_func_function(boost::bind(&proto_object::create_function,this,_1,_2)),
  m_members(superclass->m_members)
{
    register_members();
}

proto_object::proto_object(const proto_object & src)
 :m_func_value(boost::bind(&proto_object::get_member_value,this,_1)),
  m_func_super(boost::bind(&proto_object::call_super_operation,this,_1,_2)),
  m_func_function(boost::bind(&proto_object::create_function,this,_1,_2)),
  m_operation(src.m_operation),
  m_members(src.m_members)
{
    register_members();
    //WARNING - be careful not to leave old symbol registrations pointing to source
}

proto_object::proto_object(domain * aDomain)
  :m_func_value(boost::bind(&proto_object::get_member_value,this,_1)),
   m_func_super(boost::bind(&proto_object::call_super_operation,this,_1,_2)),
   m_func_function(boost::bind(&proto_object::create_function,this,_1,_2)),
   m_members(aDomain)
{
   register_members();
}

std::string proto_object::apply(std::list<std::string> & arglist,domain * aDomain)
{
    m_operation=functionN::pop_arg<std::string>(arglist);
    symbol * operation=m_members.require_symbol(m_operation,domain::SEARCH_PARENTS);
    
    bool link=false;
    if(aDomain!=m_members.get_parent()) 
    {
        m_members.link(aDomain);
        link=true;
    }
    
    std::string result;
    
    try
    {
        result=operation->apply(arglist,&m_members);
    }
    catch(const error_key &)
    {
        if(link) m_members.unlink();
        throw;
    }
    
    if(link) m_members.unlink();
    
    return result;
}

std::string proto_object::value()const
{
    throw error_key("runtime.object.no_value");
}

void proto_object::add_member(const char * id,symbol * sym)
{
    m_members.register_symbol(id,sym);
}

domain * proto_object::get_parent_domain()const
{
    return m_members.get_parent();
}

void proto_object::register_members()
{
    m_members.register_symbol("value",&m_func_value);
    m_members.register_symbol("super",&m_func_super);
    m_members.register_symbol("function",&m_func_function);
}

std::string proto_object::get_member_value(const std::string & name)const
{
    return m_members.require_symbol(name)->value();
}

std::string proto_object::call_super_operation(std::list<std::string> & arglist,domain * aDomain)
{
    domain * parent=m_members.get_parent();
    if(!parent) throw error_key("runtime.object.super_not_found");
    symbol * opsym=parent->lookup_symbol(m_operation,domain::SEARCH_PARENTS);
    if(!opsym) throw error_key("runtime.object.super_not_found");
    return opsym->apply(arglist,aDomain);
}

void proto_object::create_function(const std::string & id,const std::string & code)
{
    alias * func=new alias;
    func->push(code);
    m_members.register_symbol(id,func,domain::ADOPT_SYMBOL);
}

object_adaptor::object_adaptor(domain * aDomain):proto_object(aDomain){}
object_adaptor::object_adaptor(const object_adaptor & src):proto_object(src){}
proto_object * object_adaptor::clone()const{return new object_adaptor(*this);}

std::string null::apply(std::list<std::string> &,domain *){throw error_key("runtime.null_object");}
std::string null::value()const{throw error_key("runtime.null_object");}

} //namespace cubescript
