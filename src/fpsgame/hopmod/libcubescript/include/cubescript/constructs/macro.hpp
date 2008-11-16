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
#ifndef LIBCUBESCRIPT_CONSTRUCT_MACRO_HPP
#define LIBCUBESCRIPT_CONSTRUCT_MACRO_HPP

#include "../construct.hpp"

namespace cubescript{
namespace cons{
/*!
    The macro construct allows for expression evaluation and symbol value
    substituation - at parse time, from within a block construct. The beginning
    of a macro construct is denoted by the @ character, the number of
    consecutive @ characters indicates at which block scope the macro should be
    evaluated. This construct has two known convenient uses: passing arguments
    to anonymous aliases and formatting strings.
    
    The following code example shows an anonymous alias being passed to the
    sleep function where it will be executed 2 seconds after the deadline is
    initiated.
    @code
    > sleep 2000 [echo [sleep was called at @@(local_time)]]
    @endcode
    The macro construct in this example, "@@(local_time))", has an escape level
    of 2 (indicated by two @ characters) which means the macro is parsed and
    evaluated when the first block is parsed, by the time the second block
    comes to being interpreted, the macro result has already been substituted.
    The "(local_time)" part of the macro is an expression to be evaluated.
    Symbol macros have the form, "@name_of_symbol", where characters proceeding
    the @ character(s) is a word construct.
*/
class macro:public construct
{
public:
    macro();
    ~macro();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
    int get_escape_level()const;
private:
    bool parse_subt(std::istream &);
    int m_escape;
    construct * m_subt;
};

} //namespace cons
} //namespace cubescript

#endif
