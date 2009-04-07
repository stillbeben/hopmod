/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"
#include "fungu/script/function.hpp"
#include "fungu/script/variable.hpp"

namespace fungu{
namespace script{
namespace corelib{
    
namespace detail{

inline const_string at(const std::vector<const_string> & v, int i)
{
    if(i >= static_cast<int>(v.size()) || i < 0) 
        throw error(OPERATION_ERROR,boost::make_tuple(std::string("index out of bounds")));
    return v[i];
}

inline int listlen(const std::vector<const_string> & v)
{
    return static_cast<int>(v.size());
}

inline result_type foreach(env::object::apply_arguments & args,env::frame * frame)
{
    if(args.size() < 2) throw error(NOT_ENOUGH_ARGUMENTS,boost::make_tuple(2));
    call_serializer cs(args,frame);
    
    std::vector<const_string> v = cs.deserialize(args.front(), target_tag<std::vector<const_string> >());
    args.pop_front();
    
    code_block cb = cs.deserialize(args.front(), target_tag<code_block>());
    args.pop_front();
    
    result_type result;
    
    env::frame inner_frame(frame);
    
    const_string current_arg;
    managed_variable<const_string> current_arg_var(current_arg);
    current_arg_var.set_temporary_flag();
    current_arg_var.lock_write(true);
    
    inner_frame.bind_object(&current_arg_var, FUNGU_OBJECT_ID("arg1"));
    
    for(std::vector<const_string>::const_iterator it = v.begin();
         it != v.end(); it++)
    {
        current_arg = *it;
        result = cb.eval_each_expression(&inner_frame);
    }
    
    return result;
}

} //namespace detail

void register_vector_functions(env & environment)
{
    static function<const_string (const std::vector<const_string> &,int)> at_func(detail::at);
    environment.bind_global_object(&at_func, FUNGU_OBJECT_ID("at"));
    
    static function<int (const std::vector<const_string> &)> listlen_func(detail::listlen);
    environment.bind_global_object(&listlen_func, FUNGU_OBJECT_ID("listlen"));
    
    static function<raw_function_type> foreach_func(detail::foreach);
    environment.bind_global_object(&foreach_func, FUNGU_OBJECT_ID("foreach"));
}

} //namespace corelib
} //namespace script
} //namespace fungu