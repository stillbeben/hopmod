/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_OBJECT_HPP
#define FUNGU_SCRIPT_CLASS_OBJECT_HPP

#include "class_interface.hpp"

namespace fungu{
namespace script{

template<typename Class>
class class_object : public env::object
{
public:
    class_object(Class * object)
     :m_object(object),
      m_interface(class_interface<Class>::get_declarator())
    {
        
    }
    
    class_object(Class * object,class_interface<Class> & interface)
     :m_object(object),m_interface(interface)
    {
        
    }
    
    result_type call(call_arguments & args,frame * aScope)
    {
        if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
        const_string methodname=args.front();
        args.pop_front();
        return m_interface.call_method(m_object,methodname,args,aScope);
    }
    
    result_type value()
    {
        throw error(NO_VALUE);
    }
private:
    Class * m_object;
    class_interface<Class> & m_interface;
};

} //namespace script
} //namespace fungu

#endif
