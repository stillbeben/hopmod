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
#ifndef LIBCUBESCRIPT_BIND_HPP
#define LIBCUBESCRIPT_BIND_HPP

#include "domain.hpp"
#include "function.hpp"
#include "constant.hpp"
#include "variable.hpp"

namespace cubescript{


template<typename T> inline
void bind(const T & c,const char * name,domain * aDomain)
{
    constant<T> * wrapper=new constant<T>(c);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

inline
void bind(const char * c,const char * name,domain * aDomain)
{
    constant<const char *> * wrapper=new constant<const char *>(c);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename T> inline
void bind(T & var,const char * name,domain * aDomain)
{
    variable_ref<T> * wrapper=new variable_ref<T>(var);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R> inline
void bind(R(&func)(),const char * name,domain * aDomain)
{
    function0<R> * wrapper=new function0<R>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1> inline
void bind(R(& func)(A1),const char * name,domain * aDomain)
{
    function1<R,A1> * wrapper=new function1<R,A1>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2> inline
void bind(R(&func)(A1,A2),const char * name,domain * aDomain)
{
    function2<R,A1,A2> * wrapper=new function2<R,A1,A2>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2,typename A3> inline
void bind(R(&func)(A1,A2,A3),const char * name,domain * aDomain)
{
    function3<R,A1,A2,A3> * wrapper=new function3<R,A1,A2,A3>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R,typename A1,typename A2,typename A3,typename A4> inline
void bind(R(&func)(A1,A2,A3,A4),const char * name,domain * aDomain)
{
    function4<R,A1,A2,A3,A4> * wrapper=new function4<R,A1,A2,A3,A4>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename R> inline
void bind(R(&func)(std::list<std::string> &,domain *),const char * name,domain * aDomain)
{
    functionV<R> * wrapper=new functionV<R>(func);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

template<typename SymbolType> inline
void bind_symbol(SymbolType & var,const char * name,domain * aDomain)
{
    SymbolType * wrapper=new SymbolType(var);
    aDomain->register_symbol(name,wrapper,domain::ADOPT_SYMBOL);
}

} //namespace cubescript

#endif
