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
#ifndef LIBCUBESCRIPT_CONSTRUCT_HPP
#define LIBCUBESCRIPT_CONSTRUCT_HPP

#include <istream>
#include <boost/noncopyable.hpp>
#include "predefs.hpp"

namespace cubescript{

/*!
    @brief The base class for construct types.

    A derived construct class represents a syntatical construct of the
    CubeScript programming language - consequently, the construct class is a
    critical design concept of the scripting engine. The primary functions of a
    construct are parsing and evaluation. The parsing function reads and
    interprets CubeScript syntax into an internal representation for later
    evaluation. The evaluation function uses the internal representation to
    perform an operation.
*/
class construct:public boost::noncopyable
{
public:
    construct();
    virtual ~construct();

    /*!
        @brief Parse CubeScript code.
        @return The return value is True when parsing is complete, otherwise
                False is the return value.

        The parse method is stream-oriented; when the current function call
        cannot complete the parsing, because not enough input has been given
        yet, the function will return to the caller, who will be responsible to
        re-call the function when more input data is received - the parsing
        state is preserved among multiple function calls until parsing is 
        complete.

        When a parsing error is detected an exception of syntax_error type is 
        thrown.
    */
    virtual bool parse(std::istream & input)=0;
    
    /*!
        @brief Evaluate the parsed construct.
        @brief The result of the evaluation.
    */
    virtual std::string eval()=0;
    
    /*!
        @brief Returns a reformed representation of the parsed construct.
    */
    virtual std::string formed()const=0;

    /*!
        @brief Set the execution/symbol domain.
        @param d pointer to the domain object.
        
        This method affects symbol resolution local to the construct object. It
        is convention for derived construct types to pass their domain
        pointer to their sub construct objects - a child construct inherits its
        parent's symbol domain.
        
        You should call this method before calling the parse method.
        
        @see domain
    */
    void set_domain(domain * d);
    
    domain * get_domain()const;
private:
    domain * m_ctx;
};

} //namespace cubescript

#endif
