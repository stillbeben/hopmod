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

#include <stack>
#include <sstream>
#include <boost/bind.hpp>
#include "cubescript/alias.hpp"
#include "cubescript/error.hpp"
#include "cubescript/lexcast.hpp"
#include "cubescript/domain.hpp"
#include "cubescript/reference.hpp"
#include "cubescript/function.hpp"
#include "cubescript/constructs/expression.hpp"
#include "cubescript/bind.hpp"

namespace cubescript{

alias::alias()
 :m_local(NULL)
{
    
}

void alias::push(const std::string & block)
{
    m_blocks.push(block);
}

bool alias::pop()
{
    if(m_blocks.empty()) return false;
    m_blocks.pop();
    return true;
}

void alias::result(std::string str)
{
    m_result=str;
    m_set_result=true;
}

std::string alias::apply(std::list<std::string> & args,domain * parent_domain)
{
    if(m_blocks.empty()) return "";
    
    domain local_domain(parent_domain,domain::TEMPORARY_DOMAIN);
    m_local=&local_domain;
    
    bind(args.size(),"numargs",&local_domain);
    
    std::list<alias> arglist;
    char argname[5]={'a','r','g','0','\0'};
    int argi=1;
    
    while(!args.empty())
    {
        alias argx;
        argx.push(args.front());
        arglist.push_back(argx);
        args.pop_front();
        
        argname[3]='0'+(argi++);
        
        local_domain.register_symbol(std::string(argname),&(arglist.back()));
    }
    
    function1<void,std::string> result_func(std::bind1st(std::mem_fun(&alias::result),this));
    local_domain.register_symbol("result",&result_func);
    
    function2<void,const std::string &,const std::string &> local_func(boost::bind(&alias::create_sub_alias,this,m_local,_1,_2));
    local_domain.register_symbol("local",&local_func);
    
    function2<void,const std::string &,const std::string &> my_func(boost::bind(&alias::create_my_variable,this,_1,_2));
    local_domain.register_symbol("my",&my_func);
    
    function2<void,const std::string &,const std::string &> reference_func(boost::bind(&alias::create_reference,this,m_local,m_local,_1,_2));
    local_domain.register_symbol("reference",&reference_func);
    
    functionV<void> parameters_func(boost::bind(&alias::create_parameter_refs,this,_1,_2));
    local_domain.register_symbol("parameters",&parameters_func);
    
    std::string eval_result=eval_block(m_local);
    
    return eval_result;
}

std::string alias::value()const
{
    if(m_blocks.empty()) return "";
    return m_blocks.top();
}

std::string alias::eval_block(domain * parent_domain)
{
    //a temporary non-thread-safe solution to stack overflow from infinite recursion.
    static int depth=0;
    if(depth++>1000)
    {
        depth=0;
        throw error_key("runtime.recursion.limit_exceeded");
    }
    
    m_set_result=false;
    
    std::string result;
    
    std::stringstream code(m_blocks.top());
    code.seekp(0,std::ios_base::end);
    code<<"\r\n";
    
    while(code.peek()>0)
    {
        cons::expression statement;
        statement.set_domain(parent_domain);
        
        bool done=statement.parse(code);
        
        if(!done)
        {
            --depth;
            throw error_key("syntax.expected_terminator.expression");
        }
        
        if(!statement.is_empty()) result=statement.eval();
        
        char lastchar;
        code.get(lastchar);
        
        if(lastchar=='\r') code.get(); //get lf
    }
    
    --depth;
    
    if(m_set_result) result=m_result;
    
    return result;
}

std::string alias::save(const std::string & id)const
{
    std::ostringstream form;
    form<<id<<" = ["<<value()<<"]";
    return form.str();
}

void alias::create_sub_alias(domain * aDomain,const std::string & name,const std::string & code)
{
    alias * subAlias=aDomain->get_or_create_symbol<alias>(name,alias(),domain::SEARCH_LOCAL_ONLY);
    subAlias->pop();
    subAlias->push(code);
}

void alias::create_reference(domain * lookupDomain,domain * registerDomain,const std::string & refname,const std::string & dstname)
{
    symbol * dst=lookupDomain->lookup_symbol(dstname);
    if(!dst) throw error_key("runtime.unknown_symbol");
    if(dynamic_cast<reference *>(dst)) throw error_key("runtime.reference.refers_to_reference");

    registerDomain->get_or_create_symbol<reference>(refname,reference(*dst),domain::SEARCH_LOCAL_ONLY);
}

void alias::create_parameter_refs(std::list<std::string> & arglist,domain *)
{
    if(arglist.size() > parse_type<unsigned int>(m_local->require_symbol("numargs")->value())) throw error_key("runtime.function.parameters.not_enough_passed");
    char argname[]={'a','r','g','0','\0'};
    for(int i=1; !arglist.empty(); ++i)
    {
        argname[3]='0'+i;
        create_reference(m_local,m_local,arglist.front(),argname);
        arglist.pop_front();
    }
}

void alias::create_my_variable(const std::string & name,const std::string & value)
{
    alias * subAlias=m_local->get_or_create_symbol<alias>(name,alias(),domain::HIDE_FROM_DESCENDANTS);
    subAlias->pop();
    subAlias->push(value);
}

alias_function::alias_function(const std::string & code,domain * aDomain)
 :m_domain(aDomain)
{
   m_alias.push(code);
}

std::string alias_function::run(std::list<std::string> & args)
{
    return m_alias.apply(args,m_domain);
}

} //namespace cubescript
