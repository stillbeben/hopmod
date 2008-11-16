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
#ifndef LIBCUBESCRIPT_CONSTRUCT_STRING_HPP
#define LIBCUBESCRIPT_CONSTRUCT_STRING_HPP

#include <string>
#include "../construct.hpp"

namespace cubescript{
namespace cons{
/*!
    The String construct is a character sequence enclosed in double quotes, and
    is similar to the word construct in that its evaluation returns the parse
    input, excluding the double quotes. The string construct is useful for
    supporting string literals, which may contain characters that would cause
    syntax interpretation. Parsing is terminated by a double-quote or newline
    character.
*/
class string:public construct
{
public:
    string();
    /*!
        An extension to the CubeScript language, not found in Sauerbraten's
        implementation of CubeScript, is support for the escape character. The
        escape character ("\") allows a double quote character to be expressed
        literally, any character proceeding a blackslash is treated literally.
        
        @note Escape sequences are not supported, this is using metacharacters
              to generate characters (e.g. "\n").
    */
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    std::string m_str;
};
} //namespace cons

std::string compose_string(const std::string &);

} //namespace cubescript

#endif
