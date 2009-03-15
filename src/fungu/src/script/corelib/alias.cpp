/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"

#include "fungu/script/corelib/alias.hpp"

#include "fungu/script/code_block.hpp"
#include "fungu/script/variable.hpp"
#include "fungu/script/constant.hpp"
#include "fungu/script/function.hpp"
#include <list>
#include <boost/shared_ptr.hpp>

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

class alias:public env::object
{
public:
    alias()
     :m_recursion_depth(0)
    {
        
    }
    
    void push_block(const_string code,env::frame * aScope)
    {
        m_blocks.push_front(
            boost::shared_ptr<code_block>(new code_block(code,aScope->get_env()->get_source_context()))
        );
    }
    
    bool pop_block()
    {
        if(m_blocks.empty())return false;
        m_blocks.pop_front();
        return true;
    }
    
    result_type apply(apply_arguments & args,env::frame * aScope)
    {
        if(++m_recursion_depth > aScope->get_env()->get_recursion_limit())
            throw error(HIT_RECURSION_LIMIT,boost::make_tuple(aScope->get_env()->get_recursion_limit()));
        
        if(m_blocks.empty()) return any::null_value();
        
        env::frame alias_scope(aScope);
        
        static env::frame::frame_symbol ** numargs_symbol 
            = env::register_symbol(FUNGU_OBJECT_ID("numargs"));
        constant<int> numargs(args.size());
        numargs.set_temporary_flag();
        alias_scope.bind_object(&numargs,numargs_symbol);
        
        static env::frame::frame_symbol ** params_symbol
            = env::register_symbol(FUNGU_OBJECT_ID("params"));
        function<raw_function_type> params_function(&alias::params_scriptfun);
        params_function.set_temporary_flag();
        alias_scope.bind_object(&params_function,params_symbol);
        
        std::list<alias> argaliases;
        int argn = 0;
        while(!args.empty())
        {
            argaliases.push_back(alias());
            argaliases.back().push_block(lexical_cast<const_string>(args.front()),aScope);
            argaliases.back().set_temporary_flag();
            args.pop_front();
            alias_scope.bind_object(&argaliases.back(), get_arg_symbol(++argn));
        }
        
        result_type implicit_result;
        
        code_block & block = *m_blocks.front();
        block.compile(&alias_scope);
        
        try
        {
            for(code_block::iterator i=m_blocks.front()->begin();
                i != m_blocks.front()->end() && !alias_scope.has_expired(); ++i )
            {
                implicit_result = i->eval(&alias_scope);
            }
        }
        catch(...)
        {
            m_recursion_depth--;
            throw;
        }
        
        block.destroy_compilation();
        
        m_recursion_depth--;
        
        if(!alias_scope.get_result_value().empty()) 
            return alias_scope.get_result_value();
        else return implicit_result;
    }
    
    result_type value()
    {
        if(m_blocks.empty()) return any::null_value();
        return m_blocks.front()->value();
    }
    
    const source_context * get_source_context()const{return m_blocks.front()->get_source_context();}
private:
    static result_type params_scriptfun(apply_arguments & args,frame * aFrame)
    {
        class alias_ref:public object
        {
        public:
            alias_ref(object * obj)
             :m_object(obj)
            {
                
            }
            
            result_type apply(apply_arguments & args,frame * aFrame)
            {
                return m_object->apply(args,aFrame);
            }
            
            result_type value()
            {
                return m_object->value();
            }
        private:
            object * m_object;
        };
        
        unsigned int arg_index = 0;
        while(!args.empty())
        {
            const_string argname = args.casted_front<const_string>();
            object * alias_obj = aFrame->lookup_required_object(get_arg_symbol(++arg_index));
            aFrame->bind_object(new alias_ref(alias_obj), argname.copy()).adopt_object();
            args.pop_front();
        }
        
        return any::null_value();
    }
    
    static frame::frame_symbol ** get_arg_symbol(unsigned int i)
    {
        static frame::frame_symbol ** argn_symbol[] = {
            env::register_symbol(FUNGU_OBJECT_ID("arg0")),
            env::register_symbol(FUNGU_OBJECT_ID("arg1")),
            env::register_symbol(FUNGU_OBJECT_ID("arg2")),
            env::register_symbol(FUNGU_OBJECT_ID("arg3")),
            env::register_symbol(FUNGU_OBJECT_ID("arg4")),
            env::register_symbol(FUNGU_OBJECT_ID("arg5")),
            env::register_symbol(FUNGU_OBJECT_ID("arg6"))
        };
        if(i>0 && i < sizeof(argn_symbol)/sizeof(frame::frame_symbol **))
            return argn_symbol[i];
        else return env::register_symbol(std::string("arg") + lexical_cast<std::string>(i));
    }
    
    std::list<boost::shared_ptr<code_block> > m_blocks;
    int m_recursion_depth;
};

inline result_type define_alias(env::object::apply_arguments & args,env::frame * aScope)
{
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    const_string content = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * aAlias = new alias;
    aAlias->push_block(content,aScope);
    
    aScope->bind_global_object(aAlias, name.copy())
        .adopt_object().allow_rebind();
    
    return aAlias->value();
}

inline result_type push_alias_block(env::object::apply_arguments & args,env::frame * aFrame)
{
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    const_string content = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * a = dynamic_cast<alias *>(aFrame->lookup_required_object(name));
    if(!a) throw error(BAD_CAST);
    
    a->push_block(content,aFrame);
    
    return any::null_value();
}

inline result_type pop_alias_block(env::object::apply_arguments & args,env::frame * aFrame)
{   
    const_string name = args.safe_casted_front<const_string>();
    args.pop_front();
    
    alias * a = dynamic_cast<alias *>(aFrame->lookup_required_object(name));
    if(!a) throw error(BAD_CAST);
    
    return a->pop_block();
}

} //namespace detail

void register_alias_functions(env & environment)
{
    static function<raw_function_type> define_alias_func(detail::define_alias);
    environment.bind_global_object(&define_alias_func,FUNGU_OBJECT_ID("alias"));
    
    environment.bind_infix_operator(FUNGU_OBJECT_ID("="),FUNGU_OBJECT_ID("alias"));
    
    static function<raw_function_type> push_alias_block_func(detail::push_alias_block);
    environment.bind_global_object(&push_alias_block_func,FUNGU_OBJECT_ID("push"));
    
    static function<raw_function_type> pop_alias_block_func(detail::pop_alias_block);
    environment.bind_global_object(&pop_alias_block_func,FUNGU_OBJECT_ID("pop"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
