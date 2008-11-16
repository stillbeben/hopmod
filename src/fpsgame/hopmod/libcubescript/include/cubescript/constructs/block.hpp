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
#ifndef LIBCUBESCRIPT_CONSTRUCT_BLOCK_HPP
#define LIBCUBESCRIPT_CONSTRUCT_BLOCK_HPP

#include "../construct.hpp"
#include "macro.hpp"

namespace cubescript{
namespace cons{
/*!
    The block construct is useful for expressing multiline string literals. Its
    syntax is: "[" <arbitrary-string> "]". The arbitrary string is any
    characters from the ascii character set, and can include closed sub-block
    constructs (e.g. "[some string [a sub block]]"), also "@" characters are
    interpreted as part of a macro construct.

    The block and string construct are semantically the same thing, an alias
    is assigned with a value or code (it depends how you use the alias) of 
    string type. The block construct is better for expressing code.
*/
class block:public construct
{
public:
    block();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    bool parse_macro(std::istream &);
    std::string m_code;
    int m_nested;
    macro * m_macro;
};

} //namespace cons
} //namespace cubescript

#endif
