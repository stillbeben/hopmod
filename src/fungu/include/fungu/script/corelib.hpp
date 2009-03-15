/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CORELIB_HPP
#define FUNGU_SCRIPT_CORELIB_HPP

#include "corelib/maths.hpp"
#include "corelib/alias.hpp"
#include "corelib/controlflow.hpp"
#include "corelib/exec.hpp"
#include "corelib/list.hpp"
#include "corelib/anonymous_function.hpp"
#include "corelib/reference.hpp"
#include "corelib/object.hpp"
#include "corelib/string.hpp"
#include "corelib/vector.hpp"
#include "corelib/datetime.hpp"
#include "corelib/exception.hpp"

namespace fungu{
namespace script{

inline void load_corelib(env & environment)
{
    script::env::frame::register_universal_functions();
    
    script::corelib::register_math_functions(environment);
    script::corelib::register_controlflow_functions(environment);
    script::corelib::register_alias_functions(environment);
    script::corelib::register_reference_functions(environment);
    script::corelib::register_anonfunc_functions(environment);
    script::corelib::register_list_functions(environment);
    script::corelib::register_exec_functions(environment);
    script::corelib::register_object_functions(environment);
    script::corelib::register_string_functions(environment);
    script::corelib::register_vector_functions(environment);
    script::corelib::register_datetime_functions(environment);
    script::corelib::register_exception_functions(environment);
}

} //namespace script
} //namespace fungu

#endif
