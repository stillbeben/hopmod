/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CONSTANT_HPP
#define FUNGU_SCRIPT_CONSTANT_HPP

#include "env.hpp"
#include "call_serializer.hpp"

namespace fungu{
namespace script{

/**
    
*/
template<typename T>
class constant:public env::object
{
public:
    constant(const T & value)
     :m_value(value)
    {
        
    }
    
    result_type apply(apply_arguments & args,env::frame *)
    {
        throw error(NO_WRITE);
    }
    
    result_type value()
    {
        return m_value;
    }
private:
    const T m_value;
};

} //namespace script
} //namespace fungu

#endif
