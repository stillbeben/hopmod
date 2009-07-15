/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_EXECUTE_HELPERS_HPP
#define FUNGU_SCRIPT_EXECUTE_HELPERS_HPP

namespace fungu{
namespace script{

/**
    
*/
int execute_file(const char * filename, env &);

void throw_if_error(int errcode);
    
/**
    
*/
result_type execute_text(const_string code,env::frame * parent_scope);

} //namespace script
} //namespace fungu

#endif
