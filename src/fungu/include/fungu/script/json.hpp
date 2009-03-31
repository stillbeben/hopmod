/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_STRUCTURED_OBJECT_HPP
#define FUNGU_SCRIPT_STRUCTURED_OBJECT_HPP

#error "deprecated"

#include "json/tinyjson.hpp"

namespace fungu{
namespace script{

typedef ::json::grammar<char,any_traits> json;

} //namespace script
} //namespace fungu

#endif
