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
#include "cubescript.hpp"
#include <string>
#include <sstream>
#include <cassert>
#include <functional>
#include <boost/bind.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>

#include <sys/stat.h>
#include <errno.h>
#include <regex.h>
#include <time.h>

namespace cubescript{

error_key::error_key(const char * key)
 :m_key(key)
{
    
}

const char * error_key::what()const throw()
{
    return m_key;
}

symbol_error::symbol_error(const error_key & key,const std::string & id)
 :error_key(key),m_id(id)
{
    
}

symbol_error::symbol_error(const char * key,const std::string & id)
 :error_key(key),m_id(id)
{
    
}

const std::string & symbol_error::get_id()const throw()
{
    return m_id;
}

static inline bool is_numeric(const std::string & str)
{
    int i=0;
    if(str.length()>1 && str[0]=='-') i=1;
    for(; i<str.length(); ++i) 
        if((str[i]>='0' && str[i]<='9')==false && (i>0 && str[i]=='.')==false ) return false;
    return true;
}

static inline std::string extract_path(const std::string & filename)
{
    int i=filename.length()-1;
    while(i>=0 && filename[i]!='/' && filename[i]!='\\' ) --i;
    return filename.substr(0,i+1);
}

symbol::~symbol()
{
    
}

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
    boost::shared_ptr<symbol> owner_ptr;
    if(flags & ADOPT_SYMBOL) owner_ptr=boost::shared_ptr<symbol>(s);
    m_symbols[id]=symbol_pointers(s,owner_ptr);
}

symbol * domain::lookup_symbol(const std::string & id,int search)const
{
    symbol_map::const_iterator it=m_symbols.find(id);
    if(it==m_symbols.end()) //not found
    {
        if(search & SEARCH_PARENTS && m_parent) return m_parent->lookup_symbol(id,search);
        return NULL;
    }
    return it->second.first;
}

symbol * domain::require_symbol(const std::string & id,int search)const
{
    symbol * s=lookup_symbol(id,search);
    if(!s) throw symbol_error("runtime.unknown_symbol",id);
    return s;
}

void domain::save(std::ostream & output)const
{
    for(symbol_map::const_iterator it=m_symbols.begin(); it!=m_symbols.end(); ++it )
    {
        std::string name=it->first;
        symbol::persistable * sym=dynamic_cast<symbol::persistable *>(it->second.first);
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

construct::construct()
{
    m_ctx=NULL;
}

construct::~construct()
{
    
}

void construct::set_domain(domain * ctx)
{
    m_ctx=ctx;
}

domain * construct::get_domain()const
{
    return m_ctx;
}

cons::word::word()
{
    m_word.reserve(32);
}

cons::word::~word()
{
    
}

bool cons::word::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(in_set(c)) m_word+=c;
        else
        {
            input.putback(c);
            return true;
        }
    }
}

std::string cons::word::eval()
{
    return m_word;
}

std::string cons::word::formed()const
{
    return m_word;
}

bool cons::word::in_set(char c)const
{
    //excluded charset
    if( c=='\"' || c=='(' || c==')' || c=='[' || c==']' 
        || c==' ' || c=='\r' || c=='\n' || c=='\t' || c==';') return false;
    
    return true;
}

cons::string::string()
{
    m_str.reserve(64);
}

bool cons::string::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(m_str.length() && m_str[m_str.length()-1]=='\\')
            m_str.erase(m_str.end()-1);
        else if(c=='\"' || c=='\r' || c=='\n')
        {
            input.putback(c);
            
            return true;
        }
        
        m_str+=c;
    }
}

std::string cons::string::eval()
{
    return m_str;
}

std::string cons::string::formed()const
{
    return compose_string(m_str);
}

cons::macro::macro()
{
    m_escape=1;
    m_subt=NULL;
}

cons::macro::~macro()
{
    if(m_subt) delete m_subt;
}

bool cons::macro::parse(std::istream & input)
{
    if(m_subt) return parse_subt(input);
    
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='@')
        {
            ++m_escape;
        }
        else
        {
            if(c=='(') m_subt=new expression_container;
            else
            {
                input.putback(c);
                m_subt=new word;
            }
            return parse_subt(input);
        }
    }
}

std::string cons::macro::eval()
{
    bool is_word=dynamic_cast<word *>(m_subt);
    if(is_word) return get_domain()->require_symbol(m_subt->eval())->value();
    else return m_subt->eval();
}

std::string cons::macro::formed()const
{
    std::stringstream writer;
    for(int i=1; i<=m_escape; ++i) writer<<"@";
    writer<<m_subt->formed();
    return writer.str();
}

int cons::macro::get_escape_level()const
{
    return m_escape;
}

bool cons::macro::parse_subt(std::istream & input)
{
    m_subt->set_domain(get_domain());
    
    bool done=m_subt->parse(input);
    if(!done) return false;
    
    bool is_expression=dynamic_cast<expression_container *>(m_subt);
    if(is_expression)
    {
        char termc;
        input.get(termc);
        assert(!input.fail());
        if(termc!=')') throw error_key("syntax.expected_terminator.expression");
    }
    
    return true;
}

cons::block::block()
{
    m_code.reserve(256);
    m_nested=0;
    m_macro=NULL;
}

bool cons::block::parse(std::istream & input)
{
    while(true)
    {
        if(m_macro && !parse_macro(input)) return false;
        
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='@')
        {
            m_macro=new macro;
            if(!parse_macro(input)) return false;
        }
        else
        {
            if(c=='[') ++m_nested;
            else if(c==']')
            {
                if(!m_nested)
                {
                    input.putback(c);
                    return true;
                }
                --m_nested;
            }
            
            m_code+=c;
        }
    }
}

std::string cons::block::eval()
{
    return m_code;
}

std::string cons::block::formed()const
{
    std::stringstream writer;
    writer<<"["<<m_code<<"]";
    return writer.str();
}

bool cons::block::parse_macro(std::istream & input)
{
    m_macro->set_domain(get_domain());
    if(!m_macro->parse(input)) return false;
    
    if( m_macro->get_escape_level() > m_nested ) m_code+=m_macro->eval();
    else m_code+=m_macro->formed();
    
    delete m_macro;
    m_macro=NULL;
    
    return true;
}

cons::expression::expression()
{
    m_parsing=NULL;
}

cons::expression::~expression()
{
    for(std::list<construct *>::iterator it=m_ops.begin(); it!=m_ops.end(); ++it)
        delete *it;
}

bool cons::expression::parse(std::istream & input)
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

std::string cons::expression::eval()
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
                
                bool is_word=dynamic_cast<word *>(next);
                if(is_word && arg.length()>1 && arg[0]=='$')
                {
                    int resolutions;
                    for(resolutions=1; resolutions < arg.length()-1 && arg[resolutions]=='$'; resolutions++) ;
                    arg=arg.substr(resolutions);
                    symbol * s;
                    for(int i=0; i<resolutions; i++)
                    {
                        s=get_domain()->require_symbol(arg);
                        arg=s->value();
                    }
                }
                
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
                else throw symbol_error("runtime.unknown_symbol",args.front());
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
    catch(const script_error<symbol_error> &){throw;}
    catch(const script_error<error_key> &){throw;}
    catch(const expr_error<symbol_error> &){throw;}
    catch(const expr_error<error_key> &){throw;}
    catch(const symbol_error & key) {throw expr_error<symbol_error>(key,formed());}
    catch(const error_key & key){throw expr_error<error_key>(key,formed());}
}

std::string cons::expression::formed()const
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

bool cons::expression::is_empty()const
{
    return m_ops.empty() || !eval_type(*m_ops.begin());
}

bool cons::expression::parse_sub_construct(std::istream & input)
{
    assert(m_parsing);
    
    m_parsing->set_domain(get_domain());
    
    try
    {
        if(!m_parsing->parse(input)) return false;
        
        char term;
        input.get(term);
        assert(!input.fail());
        
        if(m_sub_term=='\0')
        {
            if(input.tellg()==m_inputpos) throw error_key("syntax.unexpected_terminator");
            input.putback(term);
        }
        else if(term!=m_sub_term) throw error_key("syntax.expected_terminator");
    }
    catch(script_error<symbol_error> &){throw;}
    catch(script_error<error_key> &){throw;}
    catch(symbol_error & key){throw expr_error<symbol_error>(key,formed());}
    catch(error_key & key){throw expr_error<error_key>(key,formed());}
    
    m_ops.push_back(m_parsing);
    m_parsing=NULL;
    return true;
}

bool cons::expression::in_termset(char c)const
{
    return c==';' || c==')' || c=='\r' || c=='\n';
}

bool cons::expression::assign_alias_macro(std::list<std::string> & args)
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

bool cons::expression::eval_type(const construct * c)const
{
    bool is_comment=dynamic_cast<const comment *>(c);
    return !is_comment;
}

cons::expression_container::expression_container()
{
    m_parsing=new expression;
    m_exps.push_back(m_parsing);
}

cons::expression_container::~expression_container()
{
    for(std::list<expression *>::iterator it=m_exps.begin(); it!=m_exps.end(); ++it) delete *it;
}

bool cons::expression_container::parse(std::istream & input)
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

std::string cons::expression_container::eval()
{
    std::string result;
    for(std::list<expression *>::iterator it=m_exps.begin(); it!=m_exps.end(); ++it)
        result=(*it)->eval();
    return result;
}

std::string cons::expression_container::formed()const
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

bool cons::comment::parse(std::istream & input)
{
    while(true)
    {
        char c;
        input.get(c);
        if(input.fail()) return false;
        
        if(c=='\r' || c=='\n')
        {
            if(m_line.length()<1 || m_line[0]!='/')
                throw error_key("syntax.expected_comment_token");
            input.putback(c);
            return true;
        }
        
        m_line+=c;
    }
}

std::string cons::comment::eval()
{
    return "";
}

std::string cons::comment::formed()const
{
    std::stringstream writer;
    writer<<"//"<<m_line;
    return writer.str();
}

std::string compose_string(const std::string & txt)
{
    if(txt.length()==0) return "";
    
    std::string result;
    result.reserve(txt.length()+2);
    
    result="\"";
    
    for(int i=0; i<txt.length(); ++i)
    {
        char c=txt[i];
        if(c=='\"' || c=='\r' || c=='\n') result+='\\';
        result+=c;
    }
    
    result+="\"";
    
    return result;
}

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
    
    alias numargs;

    domain local_domain(parent_domain,domain::TEMPORARY_DOMAIN);
    m_local=&local_domain;
    domain write_domain(&local_domain);
    
    local_domain.register_symbol("numargs",&numargs);
    
    numargs.push(print_type<int>(args.size()));
    
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
        
        local_domain.register_symbol(argname,&(arglist.back()));
    }
    
    function1<void,std::string> result_func(std::bind1st(std::mem_fun(&alias::result),this));
    local_domain.register_symbol("result",&result_func);
    
    function2<void,const std::string &,const std::string &> local_func(boost::bind(&alias::create_sub_alias,this,m_local,_1,_2));
    local_domain.register_symbol("local",&local_func);
    
    function2<void,const std::string &,const std::string &> reference_func(boost::bind(&alias::create_reference,this,m_local,m_local,_1,_2));
    local_domain.register_symbol("reference",&reference_func);
    
    functionV<void> parameters_func(boost::bind(&alias::create_parameter_refs,this,_1,_2));
    local_domain.register_symbol("parameters",&parameters_func);
    
    std::string eval_result=eval_block(&write_domain);
    
    if(parent_domain)
    {
        domain * dst=parent_domain->get_first_nontmp_domain();
        if(dst) dst->unite(&write_domain);
    }
    
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
    if(depth++>900)
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
    if(!dst) throw symbol_error("runtime.unknown_symbol",dstname);
    if(dynamic_cast<reference *>(dst)) throw error_key("runtime.reference.refers_to_reference");

    registerDomain->get_or_create_symbol<reference>(refname,reference(*dst),domain::SEARCH_LOCAL_ONLY);
}

void alias::create_parameter_refs(std::list<std::string> & arglist,domain *)
{
    if(arglist.size() > parse_type<int>(m_local->require_symbol("numargs")->value())) throw error_key("runtime.function.parameters.not_enough_passed");
    char argname[]={'a','r','g','0','\0'};
    for(int i=1; !arglist.empty(); ++i)
    {
        argname[3]='0'+i;
        create_reference(m_local,m_local,arglist.front(),argname);
        arglist.pop_front();
    }
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

static std::list<std::string> gs_noargs;

std::list<std::string> & args0()
{
    gs_noargs.clear();
    return gs_noargs;
}

reference::reference(symbol & aSymbol)
 :m_symbol(aSymbol)
{
    
}

std::string reference::apply(std::list<std::string> & arglist,domain * aDomain)
{
    return m_symbol.apply(arglist,aDomain);
}

std::string reference::value()const
{
    return m_symbol.value();
}

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

namespace runtime{

void assign_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    std::string code=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias());
    while(aAlias->pop()) ;
    aAlias->push(code);
}

void push_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    std::string code=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias());
    aAlias->push(code);
}

void pop_alias(std::list<std::string> & args,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(args);
    alias * aAlias=aDomain->get_or_create_symbol<alias>(id,alias());
    aAlias->pop();
}

inline int math_add(int a,int b){return a + b;}
inline int math_sub(int a,int b){return a - b;}
inline int math_mul(int a,int b){return a * b;}
inline int math_div(int a,int b){if(b==0) throw error_key("runtime.divide_by_zero"); return a / b;}
inline int math_mod(int a,int b){return a % b;}
inline int math_min(int a,int b){return a < b? a : b;}
inline int math_max(int a,int b){return a > b? a : b;}
inline float math_fadd(float a,float b){return a+b;}
inline float math_fsub(float a,float b){return a-b;}
inline float math_fmul(float a,float b){return a*b;}
inline float math_fdiv(float a,float b){return a/b;}
#include <math.h>
inline float math_round(float a){return roundf(a);}
inline float math_ceil(float a){return ceilf(a);}
inline float math_floor(float a){return floorf(a);}

inline bool comp_equal(int a,int b){return a == b;}
inline bool comp_nequal(int a,int b){return a != b;}
inline bool comp_gt(int a,int b){return a > b;}
inline bool comp_gte(int a,int b){return a >= b;}
inline bool comp_lt(int a,int b){return a < b;}
inline bool comp_lte(int a,int b){return a<= b;}

inline bool logic_not(bool a){return !a;}
inline bool logic_or(alias_function & a,alias_function & b){return parse_type<bool>(a.run(args0())) || parse_type<bool>(b.run(args0()));}
inline bool logic_and(alias_function & a,alias_function & b){return parse_type<bool>(a.run(args0())) && parse_type<bool>(b.run(args0()));}
inline bool logic_xor(bool a,bool b){return a ^ b;}

std::string control_if(std::list<std::string> & args,domain * context)
{
    bool has_true_arg=args.size()>1;
    bool has_false_arg=args.size()>2;
    
    bool cond=functionN::pop_arg<bool>(args);
    std::string true_block=has_true_arg ? functionN::pop_arg<std::string>(args) :"";
    std::string false_block=has_false_arg ? functionN::pop_arg<std::string>(args) :"";
    
    alias runner;
    
    if(cond) {if(has_true_arg) runner.push(true_block);}
    else if(has_false_arg) runner.push(false_block);
    
    std::list<std::string> noargs;
    return runner.apply(noargs,context);
}

void control_loop(std::list<std::string> & args,domain * parent_domain)
{
    std::string counter=functionN::pop_arg<std::string>(args);
    int n=functionN::pop_arg<int>(args);
    std::string body=functionN::pop_arg<std::string>(args);
    
    domain tmp_domain(parent_domain);

    alias alias_counter;
    tmp_domain.register_symbol(counter,&alias_counter);
    
    alias alias_body;
    alias_body.push(body);
    
    std::list<std::string> noargs;
    
    for(int i=0; i<n; ++i)
    {
        alias_counter.push(print_type<int>(i));
        alias_body.apply(noargs,&tmp_domain);
        alias_counter.pop();
    }
}

void control_while(std::list<std::string> & args,domain * context)
{
    alias cond;
    cond.push(functionN::pop_arg<std::string>(args));
    
    alias body;
    body.push(functionN::pop_arg<std::string>(args));
    
    std::list<std::string> noargs;
    
    while(parse_type<bool>(cond.apply(noargs,context))) body.apply(noargs,context);
}

bool predicate_symbol(std::list<std::string> & args,domain * context)
{
    std::string name=functionN::pop_arg<std::string>(args);
    symbol * s=context->lookup_symbol(name);
    return s;
}

inline
std::string list_at(std::vector<std::string> list,int n)
{
    if(n<0 || n>=list.size()) throw error_key("runtime.out_of_bounds");
    return list[n];
}

inline 
int list_len(std::vector<std::string> list)
{
    return list.size();
}

void _throw(std::string key)
{
    if(key.compare(0,8,"runtime.")!=0) throw error_key("runtime.invalid_throw");
    static std::string s_key; //FIXME not thread safe!
    s_key=key;
    throw error_key(s_key.c_str());
}

inline bool strcmp(std::string a,std::string b){return a==b;}

std::string concat(std::list<std::string> & args,domain *)
{
    std::string result;
    while(!args.empty())
    {
        result+=functionN::pop_arg<std::string>(args);
        if(args.size()>0) result+=" ";
    }
    return result;
}

std::string concatword(std::list<std::string> & args,domain *)
{
    std::string result;
    while(!args.empty()) result+=functionN::pop_arg<std::string>(args);
    return result;
}

std::string format(std::list<std::string> & args,domain *)
{
    std::string result;
    
    std::string _template=functionN::pop_arg<std::string>(args);
    
    size_t totalc=_template.length();
    
    std::vector<std::string> elements;
    while(!args.empty())
    {
        std::string el=functionN::pop_arg<std::string>(args);
        totalc+=el.length();
        elements.push_back(el);
    }
    
    result.reserve(totalc);
    
    std::string::size_type len=_template.length();
    for(int i=0; i<len; ++i)
    {
        if(_template[i]=='%')
        {
            if(i==len-1) throw error_key("runtime.function.format.invalid_template");
            char digitc=_template[i+1];
            if(!(digitc>='1' && digitc<='9')) throw error_key("runtime.function.format.invalid_template");
            int n=digitc-'1';
            if(n>=elements.size()) throw error_key("runtime.function.format.missing_template_argument");
            result+=elements[n];
            ++i;
        }
        else result+=_template[i];
    }
    
    return result;
}

std::vector<std::string> split(const std::string & str,const std::string & delims)
{
    std::vector<std::string> tokens(1);
    for(int i=0; i<str.length(); ++i)
    {
        bool match=false;
        for(int j=0; j<delims.length(); ++j)
            if(delims[j]==str[i])
            {
                match=true;
                break;
            }
        if(match) tokens.push_back(std::string());
        else tokens.back()+=str[i];
    }
    return tokens;
}

bool match(const std::string & pattern,const std::string & str)
{
    regex_t compiled;
    int compret=regcomp(&compiled,pattern.c_str(),0);
    if(compret!=0) throw error_key("runtime.function.match.regcomp_failed");
    int execret=regexec(&compiled,str.c_str(),0,NULL,0);
    if(execret==0) return true;
    else
    {
        if(execret==REG_ESPACE) throw error_key("runtime.function.match.nomem");
        return false;
    }
}

void foreach(const std::list<std::string> & list,alias_function & code)
{
    for(std::list<std::string>::const_iterator it=list.begin();
        it!=list.end();
        ++it )
    {
        arguments args;
        code.run(args & (*it));
    }
}

time_t get_time_now()
{
    return time(NULL);
}

std::string format_date(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    std::ostringstream result;
    result<<1900+fields->tm_year<<"-";
    if(fields->tm_mon < 10) result<<"0";
    result<<1+fields->tm_mon<<"-";
    if(fields->tm_mday < 10) result<<"0";
    result<<fields->tm_mday;
    return result.str();
}

std::string format_time(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    std::ostringstream result;
    if(fields->tm_hour < 10) result<<"0";
    result<<fields->tm_hour<<":";
    if(fields->tm_min < 10) result<<"0";
    result<<fields->tm_min<<":";
    if(fields->tm_sec < 10) result<<"0";
    result<<fields->tm_sec;
    
    int tz_off=fields->tm_gmtoff;
    
    if(tz_off < 0)
    {
        tz_off=abs(tz_off);
        result<<"-";
    }
    else result<<"+";
    
    int tz_hours=tz_off/3600;
    int tz_mins=(tz_off-(tz_hours*3600))/60;
    
    if(tz_hours < 10) result<<"0";
    result<<tz_hours<<":";
    
    if(tz_mins < 10) result<<"0";
    result<<tz_mins;
    
    return result.str();
}

std::string format_datetime(time_t local_timestamp)
{
    std::ostringstream result;
    result<<format_date(local_timestamp)<<"T"<<format_time(local_timestamp);
    return result.str();
}

const char * get_weekday(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    static char * days[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
    return *(days+fields->tm_wday);
}

const char * get_month(time_t local_timestamp)
{
    tm * fields=localtime(&local_timestamp);
    static char * months[]={"January","February","March","April","May","June","July","August","September","October","November","December"};
    return *(months+fields->tm_mon);
}

std::string _try(std::list<std::string> & arglist,domain * aDomain)
{
    //try <operation> <argument 1> ... <catch error block>
    std::string op=functionN::pop_arg<std::string>(arglist);
    if(arglist.empty()) throw error_key("runtime.function.try.missing_catch_argument");
    std::string catchcode=arglist.back();
    arglist.pop_back();
    try
    {
        return aDomain->require_symbol(op)->apply(arglist,aDomain);
    }
    catch(error_key & e)
    {
        arglist.clear();
        alias runcatch;
        runcatch.push(catchcode);
        arguments args;
        return runcatch.apply(args & e.what(),aDomain);
    }
}

std::string dump(std::list<std::string> & arglist,domain * aDomain)
{
    bool dump2file=false;
    std::ostringstream memory;
    std::ofstream file;
    std::ostream * output;
    
    if(arglist.size())
    {
        file.open(functionN::pop_arg<std::string>(arglist).c_str(),std::ios_base::out);
        dump2file=true;
        output=&file;
    }
    else output=&memory;
    
    aDomain->save(*output);
    
    if(!dump2file) return memory.str();
    else return "";
}

void create_object(std::list<std::string> & arglist,domain * aDomain)
{
    std::string className=functionN::pop_arg<std::string>(arglist);
    std::string objectName=functionN::pop_arg<std::string>(arglist);
    
    aDomain->register_symbol(objectName.c_str(),aDomain->get_symbol_of_kind<proto_object>(className)->clone(),domain::ADOPT_SYMBOL);
}

void delete_object(std::list<std::string> & arglist,domain * aDomain)
{
    std::string objectName=functionN::pop_arg<std::string>(arglist);
    aDomain->register_symbol(objectName.c_str(),new null,domain::ADOPT_SYMBOL);
}

std::string get_symbol_value(std::list<std::string> & arglist,domain * aDomain)
{
    std::string id=functionN::pop_arg<std::string>(arglist);
    return aDomain->require_symbol(id)->value();
}

void register_base_functions(domain * aDomain)
{
    static functionV<void> func_assign_alias(&assign_alias);
    static functionV<void> func_push_alias(&push_alias);
    static functionV<void> func_pop_alias(&pop_alias);
    
    aDomain->register_symbol("alias",&func_assign_alias);
    aDomain->register_symbol("push",&func_push_alias);
    aDomain->register_symbol("pop",&func_pop_alias);

    static function1<void,std::string> func_throw(&_throw);
    static functionV<std::string> func_try(&_try);
    
    aDomain->register_symbol("throw",&func_throw);
    aDomain->register_symbol("try",&func_try);
    
    static functionV<void> func_new(&create_object);
    static functionV<void> func_delete(&delete_object);
    
    aDomain->register_symbol("new",&func_new);
    aDomain->register_symbol("delete",&func_delete);
    
    static functionV<std::string> func_value(&get_symbol_value);
    
    aDomain->register_symbol("value",&func_value);
}

void register_core_functions(domain * aDomain)
{
    register_base_functions(aDomain);
    
    static function2<int,int,int> func_math_add(&math_add);
    static function2<int,int,int> func_math_sub(&math_sub);
    static function2<int,int,int> func_math_mul(&math_mul);
    static function2<int,int,int> func_math_div(&math_div);
    static function2<int,int,int> func_math_mod(&math_mod);
    static function2<int,int,int> func_math_min(&math_min);
    static function2<int,int,int> func_math_max(&math_max);
    static function2<float,float,float> func_math_fadd(&math_fadd);
    static function2<float,float,float> func_math_fsub(&math_fsub);
    static function2<float,float,float> func_math_fmul(&math_fmul);
    static function2<float,float,float> func_math_fdiv(&math_fdiv);
    static function1<float,float> func_math_round(&math_round);
    static function1<float,float> func_math_ceil(&math_ceil);
    static function1<float,float> func_math_floor(&math_floor);
    
    aDomain->register_symbol("+",&func_math_add);
    aDomain->register_symbol("-",&func_math_sub);
    aDomain->register_symbol("*",&func_math_mul);
    aDomain->register_symbol("div",&func_math_div);
    aDomain->register_symbol("mod",&func_math_mod);
    aDomain->register_symbol("min",&func_math_min);
    aDomain->register_symbol("max",&func_math_max);
    aDomain->register_symbol("fadd",&func_math_fadd);
    aDomain->register_symbol("fsub",&func_math_fsub);
    aDomain->register_symbol("fmul",&func_math_fmul);
    aDomain->register_symbol("fdiv",&func_math_fdiv);
    aDomain->register_symbol("round",&func_math_round);
    aDomain->register_symbol("ceil",&func_math_ceil);
    aDomain->register_symbol("floor",&func_math_floor);
    
    static function2<bool,int,int> func_comp_equal(&comp_equal);
    static function2<bool,int,int> func_comp_nequal(&comp_nequal);
    static function2<bool,int,int> func_comp_gt(&comp_gt);
    static function2<bool,int,int> func_comp_gte(&comp_gte);
    static function2<bool,int,int> func_comp_lt(&comp_lt);
    static function2<bool,int,int> func_comp_lte(&comp_lte);
    
    aDomain->register_symbol("=",&func_comp_equal);
    aDomain->register_symbol("!=",&func_comp_nequal);
    aDomain->register_symbol(">",&func_comp_gt);
    aDomain->register_symbol(">=",&func_comp_gte);
    aDomain->register_symbol("<",&func_comp_lt);
    aDomain->register_symbol("<=",&func_comp_lte);
    
    static function1<bool,bool> func_logic_not(&logic_not);
    static function2<bool,alias_function &,alias_function &> func_logic_or(&logic_or);
    static function2<bool,alias_function &,alias_function &> func_logic_and(&logic_and);
    static function2<bool,bool,bool> func_logic_xor(&logic_xor);
    
    aDomain->register_symbol("!",&func_logic_not);
    aDomain->register_symbol("||",&func_logic_or);
    aDomain->register_symbol("&&",&func_logic_and);
    aDomain->register_symbol("^",&func_logic_xor);
    
    static functionV<std::string> func_control_if(&control_if);
    static functionV<void> func_control_loop(&control_loop);
    static functionV<void> func_control_while(&control_while);
    
    aDomain->register_symbol("if",&func_control_if);
    aDomain->register_symbol("loop",&func_control_loop);
    aDomain->register_symbol("while",&func_control_while);
    
    static functionV<bool> func_predicate_symbol(&predicate_symbol);
    
    aDomain->register_symbol("symbol?",&func_predicate_symbol);
    
    static function2<std::string,std::vector<std::string>,int> func_list_at(&list_at);
    static function1<int,std::vector<std::string> > func_list_len(&list_len);
    static function2<void,const std::list<std::string>,alias_function &> func_foreach(&foreach);
    
    aDomain->register_symbol("at",&func_list_at);
    aDomain->register_symbol("listlen",&func_list_len);
    aDomain->register_symbol("foreach",&func_foreach);
    
    static function2<bool,std::string,std::string> func_strcmp(&strcmp);
    static functionV<std::string> func_concat(&concat);
    static functionV<std::string> func_concatword(&concatword);
    static functionV<std::string> func_format(&format);
    static function2<std::vector<std::string>,const std::string &,const std::string &> func_split(&split);
    static function2<bool,const std::string &,const std::string &> func_match(&match);
    
    aDomain->register_symbol("strcmp",&func_strcmp);
    aDomain->register_symbol("concat",&func_concat);
    aDomain->register_symbol("concatword",&func_concatword);
    aDomain->register_symbol("format",&func_format);
    aDomain->register_symbol("split",&func_split);
    aDomain->register_symbol("match",&func_match);
    
    static const char * package=PACKAGE;
    static const char * package_string=PACKAGE_STRING;
    static const char * package_version=PACKAGE_VERSION;
    static const char * package_bugreport=PACKAGE_BUGREPORT;
    
    static constant<const char *> const_engine(package);
    static constant<const char *> const_engine_string(package_string);
    static constant<const char *> const_engine_version(package_version);
    static constant<const char *> const_engine_bugreport(package_bugreport);

    aDomain->register_symbol("SCRIPT_ENGINE",&const_engine);
    aDomain->register_symbol("SCRIPT_ENGINE_STRING",&const_engine_string);
    aDomain->register_symbol("SCRIPT_ENGINE_VERSION",&const_engine_version);
    aDomain->register_symbol("SCRIPT_ENGINE_BUGREPORT",&const_engine_bugreport);
    
    static function0<time_t> func_time(&get_time_now);
    static function1<std::string,time_t> func_format_date(&format_date);
    static function1<std::string,time_t> func_format_time(&format_time);
    static function1<std::string,time_t> func_format_datetime(&format_datetime);
    static function1<const char *,time_t> func_get_weekday(&get_weekday);
    static function1<const char *,time_t> func_get_month(&get_month);
    
    aDomain->register_symbol("now",&func_time);
    aDomain->register_symbol("date",&func_format_date);
    aDomain->register_symbol("time",&func_format_time);
    aDomain->register_symbol("datetime",&func_format_datetime);
    aDomain->register_symbol("weekday",&func_get_weekday);
    aDomain->register_symbol("month",&func_get_month);
    
    static functionV<std::string> func_dump(&dump);
    
    aDomain->register_symbol("dump",&func_dump);
    
    static object_adaptor class_core(aDomain);
    aDomain->register_symbol("core",&class_core);
}

inline
void exec(std::list<std::string> & args,domain * context)
{
    std::string filename=functionN::pop_arg<std::string>(args);
    exec_file(filename,context);
}

bool filesystem_path(const std::string & filename)
{
    struct stat info;
    if(::stat(filename.c_str(),&info)==-1)
    {
        if(errno!=ENOENT) throw error_key("runtime.function.filesystem_path.unexpected_error");
        return false;
    }
    
    return true;
}

int get_file_modified(const std::string & filename)
{
    struct stat info;
    if(::stat(filename.c_str(),&info)==-1) throw error_key("runtime.function.file_modified");
    return info.st_mtim.tv_sec;
}

void include(std::list<std::string> & args,domain * aDomain)
{
    const std::vector<std::string> & include_path=*aDomain->get_symbol_of_kind<variable<std::vector<std::string> > >("include_path");
    
    std::string filename=functionN::pop_arg<std::string>(args);
    std::string full_filename;
    
    for( std::vector<std::string>::const_iterator it=include_path.begin();
         it!=include_path.end();
         ++it )
    {
        full_filename = *it + "/" + filename;
        if(filesystem_path(full_filename)) break;
        full_filename.clear();
    }
    
    if(full_filename.empty()) throw error_key("runtime.function.include.file_not_found");
    
    exec_file(full_filename,aDomain);
}

inline
char * getenv(const std::string & name)
{
    return ::getenv(name.c_str());
}

inline
bool setenv(const std::string & name,const std::string & value)
{
    return ::setenv(name.c_str(),value.c_str(),true)==0;
}

inline void unsetenv(const std::string & name)
{
    ::unsetenv(name.c_str());
}

std::string system_exec(const std::string & filename)
{
    FILE * file=popen(filename.c_str(),"r");
    if(!file) throw error_key("runtime.function.system_exec.failed");
    
    std::ostringstream result;
    
    char readbuffer[255];
    while(!feof(file))
    {
        size_t readlen=fread(readbuffer,1,sizeof(readbuffer)-1,file);
        readbuffer[readlen]='\0';
        
        result<<readbuffer;
    }
    
    pclose(file);
    
    return result.str();
}

void register_system_functions(domain * aDomain)
{
    static functionV<void> func_exec(&exec);
    aDomain->register_symbol("exec",&func_exec);
    
    variable< std::vector<std::string> > * var_include_path=new variable<std::vector<std::string> >;
    aDomain->register_symbol("include_path",var_include_path,domain::ADOPT_SYMBOL);
    
    static functionV<void> func_include(&include);
    aDomain->register_symbol("include",&func_include);
    
    static function1<bool,const std::string &> func_path(&filesystem_path);
    static function1<int,const std::string &> func_file_modified(&get_file_modified);
    
    aDomain->register_symbol("path?",&func_path);
    aDomain->register_symbol("file_modified",&func_file_modified);
    
    static function1<char *,const std::string &> func_getenv(&getenv);
    aDomain->register_symbol("getenv",&func_getenv);
    
    static function2<bool,const std::string &,const std::string &> func_setenv(&setenv);
    aDomain->register_symbol("setenv",&func_setenv);
    
    static function1<void,const std::string &> func_unsetenv(&unsetenv);
    aDomain->register_symbol("unsetenv",&func_unsetenv);
    
    static function1<std::string,const std::string &> func_system(&system_exec);
    aDomain->register_symbol("system",&func_system);
    
    #ifdef OS_PLATFORM
        static const char * os_platform=OS_PLATFORM;
    #else
        static const char * os_platform="";
    #endif
    
    static constant<const char *> const_os_platform(os_platform);
    
    aDomain->register_symbol("OS_PLATFORM",&const_os_platform);
}

} //namespace runtime

std::string exec_block(const std::string & input,domain * d)
{
    std::list<std::string> noargs;
    alias runner;
    runner.push(input);
    return runner.apply(noargs,d);
}

std::string exec_script(std::istream & input,const std::string & filename,domain * aDomain)
{
    std::string result;
    int line_number=0;
    
    cons::expression * exp=NULL;
    
    while(!input.eof())
    {
        std::string line;
        std::getline(input,line);
        line+="\r\n";
        ++line_number;
        
        std::stringstream line_stream(line);
        
        while(line_stream.peek()>0)
        {
            try
            {
                if(!exp)
                {
                    exp=new cons::expression;
                    exp->set_domain(aDomain);
                }
                
                if(exp->parse(line_stream))
                {
                    result=exp->eval();
                    
                    char expterm=line_stream.get();
                    if(expterm=='\r') line_stream.get(); //lf;
                    
                    delete exp;
                    exp=NULL;
                }
            }
            catch(expr_error<symbol_error> & key){throw script_error<symbol_error>(key,filename,line_number);}
            catch(expr_error<error_key> & key){throw script_error<error_key>(key,filename,line_number);}
        }
    }
    
    return result;
}

std::string exec_file(const std::string & filename,domain * parent_domain)
{
    std::ifstream file(filename.c_str());
    
    domain local_domain(parent_domain,domain::TEMPORARY_DOMAIN);
    domain writable_domain(&local_domain);
    
    bind(filename,"FILENAME",&local_domain);
    bind(extract_path(filename),"SCRIPT_PATH",&local_domain);
    
    if(file.fail()) throw error_key("runtime.function.exec.file_not_found");
    std::string result=exec_script(file,filename,&writable_domain);
    
    domain * dst=parent_domain->get_first_nontmp_domain();
    if(dst) dst->unite(&writable_domain);
    
    return result;
}

void consume_terminator(std::istream & input)
{
    char c=input.get();
    if(c=='\r') input.get();
}

} //namespace cubescript
