/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_VARIABLE_HPP
#define FUNGU_SCRIPT_VARIABLE_HPP

#include "env.hpp"
#include <boost/function.hpp>
#include <sstream>

namespace fungu{
namespace script{

/**
    
*/
template<typename T>
class variable:public env::object
{
public:
    variable(T & var)
     :m_var(var)
    {
        
    }
    
    object_type get_object_type()const
    {
        return DATA_OBJECT;
    }
    
    void assign(const any & value)
    {
        m_var = lexical_cast<T>(value);
    }
    
    result_type apply(apply_arguments & args,env::frame *)
    {
        m_var = args.safe_casted_front<T>();
        args.pop_front();
        try{return value();}
        catch(error){return any::null_value();}
    }
    
    result_type value()
    {
        return m_var;
    }
    
    const T & get_value()
    {
        return m_var;
    }
protected:
    T & get_var(){return m_var;}
private:
    T & m_var;
};

/**
    
*/
template<typename T>
class managed_variable:public variable<T>
{
public:
    managed_variable(T & var)
     :variable<T>(var),
      m_perms(0)
    {
        
    }
    
    void assign(const any & value)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        if(m_write_hook_func)
        {
            T tmp = any_cast<T>(value);
            m_write_hook_func(tmp);
        }
        variable<T>::assign(value);
    }
    
    result_type apply(env::object::apply_arguments & args,env::frame * frame)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        if(m_write_hook_func)
        {
            T tmp = args.safe_casted_front<T>();
            m_write_hook_func(tmp);
        }
        return variable<T>::apply(args,frame);
    }
    
    result_type value()
    {
        if(m_perms & DENY_READ) throw error(NO_READ,boost::make_tuple());
        return variable<T>::value();
    }
    
    managed_variable<T> & lock_read(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_READ : m_perms & ~DENY_READ);
        return *this;
    }
    
    managed_variable<T> & lock_write(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_WRITE : m_perms & ~DENY_WRITE);
        return *this;
    }
    
    template<typename WriteHookFunction>
    managed_variable<T> & set_write_hook(WriteHookFunction func)
    {
        m_write_hook_func = func;
        return *this;
    }
private:
    enum
    {
        DENY_READ = 1,
        DENY_WRITE = 2
    };
    char m_perms;
    
    boost::function1<void,const T &> m_write_hook_func;
};

namespace var_hooks{

template<typename T>
void inclusive_range(const T & min,const T & max, const T & value)
{
    if(value < min)
    {
        std::stringstream msg;
        msg<<"value too small ("<<min<<" minimum)";
        throw error(INVALID_VALUE,boost::make_tuple(msg.str()));
    }
    else if(value > max)
    {
        std::stringstream msg;
        msg<<"value too big ("<<max<< " maximum)";
        throw error(INVALID_VALUE,boost::make_tuple(msg.str()));
    }
}

} //namespace var_hooks

template<typename T>
class lockable_variable:public variable<T>
{
public:
    lockable_variable(T & var)
     :variable<T>(var),
      m_perms(0)
    {
        
    }
    
    void assign(const any & value)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        variable<T>::assign(value);
    }
    
    result_type apply(env::object::apply_arguments & args,env::frame * frame)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        return variable<T>::apply(args,frame);
    }
    
    result_type value()
    {
        if(m_perms & DENY_READ) throw error(NO_READ,boost::make_tuple());
        return variable<T>::value();
    }
    
    lockable_variable<T> & lock_read(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_READ : m_perms & ~DENY_READ);
        return *this;
    }
    
    lockable_variable<T> & lock_write(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_WRITE : m_perms & ~DENY_WRITE);
        return *this;
    }
private:
    enum
    {
        DENY_READ = 1,
        DENY_WRITE = 2
    };
    char m_perms;
};

} //namespace script
} //namespace fungu

#endif
