/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LEXICAL_CAST_FWD_HPP
#define FUNGU_SCRIPT_LEXICAL_CAST_FWD_HPP

namespace fungu{
namespace script{

template<typename Target,typename Source>
Target lexical_cast(const Source &);    

} //namespace script
} //namespace fungu

#endif
