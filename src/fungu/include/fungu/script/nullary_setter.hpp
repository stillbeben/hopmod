/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_NULLARY_SETTER_HPP
#define FUNGU_SCRIPT_NULLARY_SETTER_HPP

#include "env.hpp"

namespace fungu{
namespace script{

/**
    
*/
class nullary_setter:public env::object
{
public:
    nullary_setter()
     :m_set(false)
    {
        
    }
    
    result_type apply(apply_arguments &,env::frame *)
    {
        m_set = true;
        return any::null_value();
    }
    
    result_type value()
    {
        return get_shared_ptr();
    }
    
    void reset()
    {
        m_set = false;
    }
    
    bool is_set()const{return m_set;}
private:
    bool m_set;
};

} //namespace script
} //namespace fungu

#endif
