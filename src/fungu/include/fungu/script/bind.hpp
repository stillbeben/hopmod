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
#include "any_variable.hpp"
#include "function.hpp"

namespace fungu{
namespace script{

template<typename T>
inline void bind_global_const(const T & value, const_string id, env & environ)
{
    constant<T> * constant_obj = new constant<T>(value);
    constant_obj->set_adopted();
    environ.bind_global_object(constant_obj, id);
}

inline void bind_global_const(const char * value,const_string id, env & environ)
{
    constant<const char *> * cstr_obj = new constant<const char *>(value);
    cstr_obj->set_adopted();
    environ.bind_global_object(cstr_obj, id);
}

template<typename T>
inline void bind_global_var(T & ref, const_string id, env & environ)
{
    variable<T> * variable_obj = new variable<T>(ref);
    variable_obj->set_adopted();
    environ.bind_global_object(variable_obj, id);
}

template<typename T>
inline void bind_global_ro_var(T & ref, const_string id, env & environ)
{
    lockable_variable<T> * varobj = new lockable_variable<T>(ref);
    varobj->set_adopted();
    varobj->lock_write(true);
    environ.bind_global_object(varobj, id);
}

template<typename T>
inline void bind_global_wo_var(T & ref, const_string id, env & environ)
{
    lockable_variable<T> * varobj = new lockable_variable<T>(ref);
    varobj->set_adopted();
    varobj->lock_read(true);
    environ.bind_global_object(varobj, id);
}

template<typename T,typename Functor>
inline void bind_global_funvar(Functor fun, const_string id,env & environ)
{
    function_variable<T> * varobj = new function_variable<T>(fun);
    varobj->set_adopted();
    environ.bind_global_object(varobj, id);
}

template<typename Signature,typename Functor>
inline void bind_global_func(Functor fun, const_string id, env & environ,const std::vector<any> * default_args = NULL)
{
    function<Signature> * function_obj = new function<Signature>(fun, default_args);
    function_obj->set_adopted();
    environ.bind_global_object(function_obj, id);
}

} //namespace script
} //namespace fungu

#endif
