/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_BIND_HPP
#define FUNGU_SCRIPT_BIND_HPP

#include "env.hpp"
#include "constant.hpp"
#include "variable.hpp"
#include "function.hpp"

namespace fungu{
namespace script{

template<typename T>
inline void bind_global_const(const T & value, const_string id, env & environ)
{
    constant<T> * constant_obj = new constant<T>(value);
    environ.bind_global_object(constant_obj, id).adopt_object();
}

inline void bind_global_const(const char * value,const_string id, env & environ)
{
    constant<const char *> * cstr_obj = new constant<const char *>(value);
    environ.bind_global_object(cstr_obj, id).adopt_object();
}

template<typename T>
inline void bind_global_var(T & ref, const_string id, env & environ)
{
    variable<T> * variable_obj = new variable<T>(ref);
    environ.bind_global_object(variable_obj, id).adopt_object();
}

template<typename Signature,typename Functor>
inline void bind_global_func(Functor fun, const_string id, env & environ)
{
    function<Signature> * function_obj = new function<Signature>(fun);
    environ.bind_global_object(function_obj, id).adopt_object();
}

} //namespace script
} //namespace fungu

#endif
