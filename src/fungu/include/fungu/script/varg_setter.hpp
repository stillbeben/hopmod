/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_VARG_SETTER_HPP
#define FUNGU_SCRIPT_VARG_SETTER_HPP

#include "env.hpp"

namespace fungu{
namespace script{

class varg_setter:public env::object
{
public:
    varg_setter()
      :m_set(false)
    {
        
    }
    
    result_type apply(apply_arguments & args,env::frame * aFrame)
    {
        while(!args.empty())
        {
            m_args.push_back(args.front());
            args.pop_front();
        }
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
        m_args.reset();
    }
    
    bool is_set()const{return m_set;}
    apply_arguments & get_args(){return m_args;}
private:
    bool m_set;
    apply_arguments m_args;
};

} //namespace script
} //namespace fungu

#endif
