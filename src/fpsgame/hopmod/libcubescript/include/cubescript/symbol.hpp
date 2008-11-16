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
#ifndef LIBCUBESCRIPT_SYMBOL_HPP
#define LIBCUBESCRIPT_SYMBOL_HPP

#include <list>
#include <string>
#include "predefs.hpp"

namespace cubescript{

/*!
    @brief The base class for symbol types.

    A derived symbol object acts as an adapter between the native and
    CubeScript runtime, performing the necessary string-type conversions and
    mapping operations. Symbol types are unintrusive to the native mapping's
    interface, meaning there are no signs of registered functions or variables
    in C++ being accessible from a scripting language.
*/
class symbol
{
public:
    virtual ~symbol();
    
    typedef std::list<std::string> arglist_type;
    
    /*!
        @brief Apply the list of arguments to the symbol.
    */
    virtual std::string apply(std::list<std::string> & args,domain * d)=0;
    
    /*!
        @brief Return a string representation of the symbol value.
    
        This method is called by the expression evaluator when the symbol name
        is prefixed by $.
    */
    virtual std::string value()const=0;
    
    class persistable;
};

class symbol::persistable:public symbol
{
public:
    virtual std::string save(const std::string &)const=0;
};

} //namespace cubescript

#endif
