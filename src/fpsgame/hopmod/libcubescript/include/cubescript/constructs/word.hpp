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
#ifndef LIBCUBESCRIPT_CONSTRUCT_WORD_HPP
#define LIBCUBESCRIPT_CONSTRUCT_WORD_HPP

#include <string>
#include "../construct.hpp"

namespace cubescript{
namespace cons{
/*!
    The Word construct is the simplest construct defined in the scripting
    engine as no interpretation is performed and evaluation returns the same
    character sequence passed to the parse function. There are no starting
    characters to denote word construct parsing and is terminated by
    characters belonging to foreign constructs. The word construct is useful
    for self-evaluating forms, i.e. symbol identifiers and numbers.
*/
class word:public construct
{
public:
    word();
    ~word();
    /*!
        Parsing is terminated by the following characters: '"', '(', ')', '[',
        ']', ';', whitespaces and newline.

        @note Useful characters that can be parsed include: '-', '?'
    */
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    inline bool in_set(char)const;
    std::string m_word;
};
} //namespace cons
} //namespace cubescript

#endif
