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
#ifndef LIBCUBESCRIPT_VARIABLE_HPP
#define LIBCUBESCRIPT_VARIABLE_HPP

#include <sstream>
#include "predefs.hpp"
#include "error.hpp"
#include "symbol.hpp"
#include "function.hpp" //needed because cstr_variable uses functionN::pop_arg.
#include "lexcast.hpp"

namespace cubescript{

/*!
    @brief The base class for variable symbol types.
    
    A variable symbol object refers to a native variable in the host
    programming language, operations in the scripting language are translated
    into native operations in the host environment.
*/
template<typename T>
class variable_base:public symbol::persistable
{
public:
    variable_base():m_ro(false){}
    
    /*!
        @brief Assign value to variable.

        Expects one argument only, providing any more arguments will result in
        an exception being thrown. The value passed is expected to have a
        string representation that can be lexically casted to the variable's
        type, incompatible notation will result in an exception being thrown.
    */
    std::string apply(std::list<std::string> & args,domain *)
    {
        if(m_ro) throw error_key("runtime.variable.read_only.");
        if(args.empty()) throw error_key("syntax.missing_arguments");
        get_ref()=parse_type<T>(args.front());
        args.pop_front();
        return "";
    }
    
    operator const T &()const{return get_ref();}
    /*!
        @brief Get the stored value.
    */
    std::string value()const{return print_type<T>(get_ref());}
    
    std::string save(const std::string & id)const
    {
        std::ostringstream form;
        form<<id<<" "<<value();
        return form.str();
    }
    
    void readonly(bool ro)
    {
        m_ro=ro;
    }
protected:
    virtual T & get_ref()const=0;
private:
    bool m_ro;
};

/*!
    @brief Variable Symbol.
    
    Is the actual variable and registerable symbol type.
*/
template<typename T>
class variable:public variable_base<T>
{
public:
    typedef T var_type;
    variable():m_ref(m_var){}
    variable(const T & value):m_var(value),m_ref(m_var){}
    variable<T> & operator=(const T & value)
    {
        get_ref()=value;
        return *this;
    }
protected:
    T & get_ref()const{return m_ref;}
private:
    T m_var;
    T & m_ref;
};

/*!
    @brief Variable Symbol.

    References an existing variable.
*/
template<typename T>
class variable_ref:public variable_base<T>
{
public:
    variable_ref(T & ref):m_var(ref){}
protected:
    T & get_ref()const{return m_var;}
private:
    T & m_var;
};

/*!
    @brief Bounded variable symbol.
    
    An exception is thrown when an out-of-bounds value is attempted to be
    assigned, in such a case, the value stored previously is restored.
*/
template<typename T,T min,T max>
class bounded_variable:public variable_ref<T>
{
public:
    bounded_variable(T & ref):variable_ref<T>(ref){}
    
    std::string apply(std::list<std::string> & args,domain * d)
    {
        T tmp=variable_ref<T>::get_ref();
        variable_ref<T>::apply(args,d);
        if( variable_ref<T>::get_ref() < min || variable_ref<T>::get_ref() > max )
        {
            variable_ref<T>::get_ref()=tmp;
            throw error_key("runtime.out_of_bounds");
        }
        return "";
    }
};

/*!
    @brief A variable symbol class for C strings.
*/
class cstr_variable:public symbol::persistable
{
public:
    cstr_variable(char * str,size_t maxlen):m_str(str),m_maxlen(maxlen),m_ro(false){}
    cstr_variable(const char * str,size_t maxlen)
    {
        m_str = const_cast<char *>(str);
        m_maxlen = maxlen;
        m_ro = true;
    }
    std::string apply(std::list<std::string> & args,domain *)
    {
        if(m_ro) throw error_key("runtime.variable.read_only");
        std::string newstr=functionN::pop_arg<std::string>(args);
        if(newstr.length() > m_maxlen-1) throw error_key("runtime.string.too_long");
        strcpy(m_str,newstr.c_str());
        return "";
    }
    std::string value()const{return m_str;}
    std::string save(const std::string & id)const
    {
        std::ostringstream form;
        form<<id<<" "<<m_str;
        return form.str();
    }
    void readonly(bool ro){m_ro=ro;}
private:
    char * m_str;
    size_t m_maxlen;
    bool m_ro;
};

} //namespace cubescript

#endif
