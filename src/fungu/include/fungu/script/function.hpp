/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_FUNCTION_HPP
#define FUNGU_SCRIPT_FUNCTION_HPP

#include "env.hpp"
#include "../dynamic_caller.hpp"
#include "../dynamic_call.hpp"
#include "call_serializer.hpp"

#ifdef FUNGU_WITH_LUA
#include "lua/arguments.hpp"
#endif

#include <boost/function.hpp>
#include <vector>

namespace fungu{
namespace script{

/**
    
*/
template<typename Signature>
class function:public env::object
{
public:
    template<typename Functor>
    function(Functor aFunctor)
     :m_function(aFunctor),m_default_args(NULL)
    {
        
    }
    
    template<typename Functor>
    function(Functor fun,const std::vector<any> * default_args)
     :m_function(fun),m_default_args(default_args)
    {
        
    }
    
    object_type get_object_type()const
    {
        return FUNCTION_OBJECT;
    }
    
    result_type apply(apply_arguments & apply_args,frame * aFrame)
    {
        if(apply_args.size() < boost::function_traits<Signature>::arity && 
            m_default_args && 
            apply_args.size() + m_default_args->size() >= boost::function_traits<Signature>::arity)
        {
            int skip = apply_args.size() - (boost::function_traits<Signature>::arity - m_default_args->size());
            for(std::vector<any>::const_iterator it = m_default_args->begin(); it != m_default_args->end(); ++it)
                if(--skip < 0) apply_args.push_back(*it);
        }
        
        call_serializer serializer(apply_args,aFrame);
        try
        {
            return dynamic_call<Signature>(m_function,apply_args,serializer);
        }
        catch(missing_args)
        {
            throw error(NOT_ENOUGH_ARGUMENTS);
        }
    }
    
    #ifdef FUNGU_WITH_LUA
    int apply(lua_State * L)
    {
        lua::arguments args(L);
        try
        {
            return dynamic_call<Signature>(m_function,args,args);
        }
        catch(missing_args)
        {
            return luaL_error(L, "missing arguments");
        }
        catch(error err)
        {
            return luaL_error(L, err.get_error_message().c_str());
        }
        catch(error_info * errinfo)
        {
            std::string msg = errinfo->get_error().get_error_message();
            delete errinfo;
            return luaL_error(L, msg.c_str());
        }
    }
    #endif
private:
    boost::function<Signature> m_function;
    const std::vector<any> * m_default_args;
};

typedef result_type (raw_function_type)(env::object::apply_arguments &,env::frame *);

template<>
class function<raw_function_type>:public env::object
{
public:
    template<typename Functor> function(Functor aFunctor):m_function(aFunctor){}
    
    result_type apply(apply_arguments & apply_args,frame * aScope)
    {
        return m_function(apply_args,aScope);
    }
private:
    boost::function<result_type (env::object::apply_arguments &,env::frame *)> m_function;
};

} //namespace script
} //namespace fungu

#endif
