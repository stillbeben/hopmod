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

#include <sstream>
#include "cubescript/constructs/expression.hpp"
#include "cubescript/constructs/word.hpp"
#include "cubescript/constructs/string.hpp"
#include "cubescript/constructs/macro.hpp"
#include "cubescript/constructs/block.hpp"
#include "cubescript/constructs/comment.hpp"
#include "cubescript/error.hpp"
#include "cubescript/domain.hpp"
#include "cubescript/symbol.hpp"
#include "cubescript/lexcast.hpp"

static bool is_numeric(const std::string & str)
{
    unsigned int i=0;
    if(str.length()>1 && (str[0]=='-' || str[0]=='+')) i=1;
    for(; i<str.length(); ++i) 
        if((str[i]>='0' && str[i]<='9')==false && (i>0 && str[i]=='.')==false ) return false;
    return true;
}

namespace cubescript{
namespace cons{

expression::expression()
{
    m_parsing=NULL;
}

expression::~expression()
{
    for(std::list<construct *>::iterator it=m_ops.begin(); it!=m_ops.end(); ++it)
        delete *it;
}

bool expression::parse(std::istream & input)
{
    while(true)
    {
        if(m_parsing) if(!parse_sub_construct(input)) return false;
        
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        m_inputpos=input.tellg();
        
        if(c==' ' || c=='\t') continue;
        
        if(in_termset(c))
        {
            input.putback(c);
            return true;
        }
        
        else
        {
            if(c=='\"')
            {
                m_sub_term='\"';
                m_parsing=new string;
            }
            else if(c=='[')
            {
                m_sub_term=']';
                m_parsing=new block;
            }
            else if(c=='(')
            {
                m_sub_term=')';
                m_parsing=new expression_container();
            }
            else if(c=='/')
            {
                m_sub_term='\0';
                m_parsing=new comment;
            }
            else
            {
                m_sub_term='\0'; //variable termination
                input.putback(c);
                m_parsing=new word;
            }
            
            if(!parse_sub_construct(input)) return false;
        }
    }
}

std::string expression::eval()
{
    std::list<std::string> args;
    
    try
    {
        for( std::list<construct *>::const_iterator it=m_ops.begin();
             it!=m_ops.end();
             ++it )
        {
            construct * next=*it;
            
            if(eval_type(next))
            {
                std::string arg=next->eval();
                args.push_back(arg);
            }
        }
        
        assign_alias_macro(args);
        
        std::string result;
        
        if(!args.empty())
        {
            symbol * s=get_domain()->lookup_symbol(args.front());
            if(!s)
            {
                if(is_numeric(args.front())) result=args.front();
                else throw error_key("runtime.unknown_symbol");
            }
            else
            {
                std::string op=args.front();
                
                args.pop_front();
                result=s->apply(args,get_domain());
                if(!args.empty()) throw error_key("syntax.too_many_arguments");
            }
        }
        
        return result;
    }
    catch(const error_key & key){throw new expr_error_context(key,this);}
    catch(error_context * frame){throw new expr_error_context(frame,this);}
}

std::string expression::formed()const
{
    std::stringstream writer;
    
    for(std::list<construct *>::const_iterator it=m_ops.begin(); it!=m_ops.end(); ++it )
    {
        bool last=it==--m_ops.end();
        
        construct * next=*it;
        writer<<next->formed()<<(!last ? " " : "");
    }

    return writer.str();
}

bool expression::is_empty()const
{
    return m_ops.empty() || !eval_type(*m_ops.begin());
}

std::string expression::get_operation_id()const throw()
{
    assert(!m_ops.empty());
    try
    {
        return m_ops.front()->eval();
    }
    catch(error_key &){return m_ops.front()->formed();}
    catch(error_context *){return m_ops.front()->formed();}
}

bool expression::parse_sub_construct(std::istream & input)
{
    assert(m_parsing);
    
    m_parsing->set_domain(get_domain());
    
    if(!m_parsing->parse(input)) return false;
    
    char term;
    input.get(term);
    assert(!input.fail());
    
    if(m_sub_term=='\0')
    {
        if(input.tellg()==m_inputpos) 
            throw error_key("syntax.unexpected_terminator"); //this
        //happens, on working code, when the parser is feed code from 
        //multiple streams.
        
        input.putback(term);
    }
    else if(term!=m_sub_term) throw error_key("syntax.expected_terminator");
    
    m_ops.push_back(m_parsing);
    m_parsing=NULL;
    return true;
}

bool expression::in_termset(char c)const
{
    return c==';' || c==')' || c=='\r' || c=='\n';
}

bool expression::assign_alias_macro(std::list<std::string> & args)
{
    if(args.size()==3 && (*(++args.begin()))=="=")
    {
        std::list<std::string>::const_iterator it=args.begin();
        std::string name=*it;
        
        ++it; ++it;
        std::string value=*it;
        
        args.clear();
        args.push_back("alias");
        args.push_back(name);
        args.push_back(value);
        
        return true;
    }
    
    return false;
}

bool expression::eval_type(const construct * c)const
{
    bool is_comment=dynamic_cast<const comment *>(c);
    return !is_comment;
}

expression_container::expression_container()
{
    m_parsing=new expression;
    m_exps.push_back(m_parsing);
}

expression_container::~expression_container()
{
    for(std::list<expression *>::iterator it=m_exps.begin(); it!=m_exps.end(); ++it) delete *it;
}

bool expression_container::parse(std::istream & input)
{
    while(true)
    {
        if(m_parsing)
        {
            m_parsing->set_domain(get_domain());
            if(!m_parsing->parse(input)) return false;
            m_parsing=NULL;
        }
        
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c==')')
        {
            input.putback(c);
            return true;
        }
        
        if(c==';' || c=='\r' || c=='\n') continue;
        
        input.putback(c);
        
        m_parsing=new expression;
        m_exps.push_back(m_parsing);
    }
}

std::string expression_container::eval()
{
    std::string result;
    for(std::list<expression *>::iterator it=m_exps.begin(); it!=m_exps.end(); ++it)
        result=(*it)->eval();
    return result;
}

std::string expression_container::formed()const
{
    std::stringstream result;
    result<<"(";
    
    bool first=true;
    for(std::list<expression *>::const_iterator it=m_exps.begin(); it!=m_exps.end(); ++it)
    {
        if(!first) result<<";";
        else first=false;
        result<<(*it)->formed();
    }
    
    result<<")";
    
    return result.str();
}

} //namespace cons

std::string expr_error_context::get_name()const{return "expr";}
std::string expr_error_context::get_description()const{return m_ctx;}

std::string expr_error_context::get_attribute(const std::string & name)const
{
    if(name=="operation") return m_opid;
    assert(false);
}

std::string deref_error_context::get_name()const{return "deref";}

std::string deref_error_context::get_description()const
{
    std::stringstream result;
    result<<"Was dereferencing identifier '"<<m_id<<"'";
    if(m_level>1) result<<" inside "<<m_level<<(m_level==1 ? " level" : " levels")<<" of indirection";
    return result.str();
}

std::string deref_error_context::get_attribute(const std::string & name)const
{
    if(name=="operation") return m_id;
    else if(name=="level") return print_type<int>(m_level);
    assert(false);
}

} //namespace cubescript
