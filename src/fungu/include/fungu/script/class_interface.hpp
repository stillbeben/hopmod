/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_INTERFACE_HPP
#define FUNGU_SCRIPT_CLASS_INTERFACE_HPP

#include "function.hpp"
#include "../dynamic_call.hpp"
#include "../member_function_traits.hpp"
#include <boost/bind/make_adaptable.hpp>
#include <stack>

namespace fungu{
namespace script{

template<typename Class>
class class_interface
{
public:
    class_interface()
     :m_member_access(&m_members),
      m_object(NULL)
    {
        
    }
    
    template<typename MemberFunction>
    class_interface<Class> & declare_method(const_string id,
        MemberFunction member_function)
    {
        typedef typename member_function_traits<MemberFunction>::function_type function_type;
        typedef typename member_function_traits<MemberFunction>::result_type return_type;
        
        m_member_access.bind_object(
            new function<function_type>(
                boost::make_adaptable<return_type>(boost::bind(member_function,boost::ref(m_object))) ),
            id)
            .adopt_object();
        
        return *this;
    }
    
    result_type call_method(Class * object,
        const_string id,
        env::object::apply_arguments args,
        env::frame * frame)
    {
        if(m_object) m_object_instances.push(m_object);
        m_object=object;
        
        result_type result=m_member_access.lookup_required_object(id)->apply(args,frame);
        
        m_object=NULL;
        if(!m_object_instances.empty()) 
        {
            m_object=m_object_instances.top();
            m_object_instances.pop();
        }
        
        return result;
    }
    
    static class_interface<Class> & get_declarator()
    {
        static class_interface<Class> instance;
        return instance;
    }
private:
    Class * m_object;
    std::stack<Class *> m_object_instances;
    env m_members;
    env::frame m_member_access;
};

} //namespace script
} //namespace fungu

#endif
