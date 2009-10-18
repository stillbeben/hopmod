/*   
 *   The Fungu Scripting Engine
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
#include "property.hpp"

#include <boost/type_traits/remove_pointer.hpp>

namespace fungu{
namespace script{

template<typename T>
inline void bind_const(const T & value, const char * id, env & environ)
{
    constant<T> * constant_obj = new constant<T>(value);
    constant_obj->set_adopted();
    environ.bind_global_object(constant_obj, const_string::literal(id));
}

inline void bind_const(const char * value, const char * id, env & environ)
{
    constant<const char *> * cstr_obj = new constant<const char *>(value);
    cstr_obj->set_adopted();
    environ.bind_global_object(cstr_obj, const_string::literal(id));
}

template<typename T>
inline void bind_var(T & ref, const char * id, env & environ)
{
    variable<T> * variable_obj = new variable<T>(ref);
    variable_obj->set_adopted();
    environ.bind_global_object(variable_obj, const_string::literal(id));
}

template<typename T>
inline void bind_ro_var(T & ref, const char * id, env & environ)
{
    lockable_variable<T> * varobj = new lockable_variable<T>(ref);
    varobj->set_adopted();
    varobj->lock_write(true);
    environ.bind_global_object(varobj, const_string::literal(id));
}

template<typename T>
inline void bind_wo_var(T & ref, const char * id, env & environ)
{
    lockable_variable<T> * varobj = new lockable_variable<T>(ref);
    varobj->set_adopted();
    varobj->lock_read(true);
    environ.bind_global_object(varobj, id);
}

template<typename T,typename Functor>
inline void bind_funvar(Functor fun, const char * id,env & environ)
{
    function_variable<T> * varobj = new function_variable<T>(fun);
    varobj->set_adopted();
    environ.bind_global_object(varobj, const_string::literal(id));
}

template<typename Signature,typename Functor>
inline void bind_func(Functor fun, const char * id, env & environ,const std::vector<any> * default_args = NULL)
{
    function<Signature> * function_obj = new function<Signature>(fun, default_args);
    function_obj->set_adopted();
    environ.bind_global_object(function_obj, const_string::literal(id));
}

template<typename Signature>
inline void bind_freefunc(Signature fun, const char * id, env & environ, const std::vector<any> * default_args = NULL)
{
    typedef typename boost::remove_pointer<Signature>::type FunctionType;
    function<FunctionType> * function_obj = new function<FunctionType>(fun, default_args);
    function_obj->set_adopted();
    environ.bind_global_object(function_obj, const_string::literal(id));
}

template<typename T, typename GetterFunction, typename SetterFunction>
inline void bind_property(GetterFunction getter, SetterFunction setter, const char * id, env & environ)
{
    property<T> * object = new property<T>(getter, setter);
    object->set_adopted();
    environ.bind_global_object(object, const_string::literal(id));
}

} //namespace script
} //namespace fungu

#endif
