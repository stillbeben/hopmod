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
#ifndef LIBCUBESCRIPT_EXEC_HELPERS_HPP
#define LIBCUBESCRIPT_EXEC_HELPERS_HPP

#include "predefs.hpp"
#include "domain.hpp"
#include "lexcast.hpp"
#include "error.hpp"

namespace cubescript{

/*!
    @brief Execute a block of CubeScript code.
    @param input a block of code (excluding the outer square brackets).
    @return eval result the string result of the last expression evaluation or
            the string argument passed to the last calling of the result
            function.
*/
std::string exec_block(const std::string & input,domain * aDomain);

/*!
    @brief Execute a script of CubeScript code.
    
    @return eval result the result string of the last expression evaluation.
    
    Expressions are parsed and evaluated until the input stream reaches EOF.
    
    The difference between this function and exec_block is on error an
    exec_failure exception is thrown, containing the line number referencing
    the bad code and a pointer to the original raised exception.
*/
std::string exec_script(std::istream & input,const std::string &,domain * aDomain);

/*!
    @brief Execute a CubeScript file.
    @return eval result the result string of the last expression evaluation.
    
    This helper function opens and passes a file input stream to exec_script.
*/
std::string exec_file(const std::string & filename,domain * aDomain);

std::list<std::string> & args0();

/*!
    @brief A helper class for building argument lists.
    
    The first parameter of the symbol apply method is an argument list for the
    apply operation. This argument list is of type std::list<std::string>, this
    helper class offers a more convenient method of forming the argument list.
    
    Example of use:
    @code
    cubescript::arguments args;
    aSymbolPtr->apply(args & 1234 & std::string("some text"),aDomainPtr);
    @endcode
*/
class arguments:public std::list<std::string>
{
public:
    arguments(){}
    template<typename A1> arguments(const A1 & a1){(*this),a1;}
    template<typename A1,typename A2> arguments(const A1 & a1,const A2 & a2){(*this),a1,a2;}
    template<typename A1,typename A2,typename A3> arguments(const A1 & a1,const A2 & a2,const A3 & a3){(*this),a1,a2,a3;}
    template<typename A1,typename A2,typename A3,typename A4> arguments(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4){(*this),a1,a2,a3,a4;}
    template<typename A1,typename A2,typename A3,typename A4,typename A5> arguments(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5){(*this),a1,a2,a3,a4,a5;}
    template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6> arguments(const A1 & a1,const A2 & a2,const A3 & a3,const A4 & a4,const A5 & a5,const A6 & a6){(*this),a1,a2,a3,a4,a5,a6;}
    template<typename T> inline arguments & operator,(const T & val)
    {
        push_back(print_type<T>(val));
        return *this;
    }
    template<typename T> inline arguments & operator&(const T & val){return ((*this),val);} //deprecated
};

class script_error_context:public error_context
{
public:
    template<typename T>
    script_error_context(const T & error,std::string file,int line):error_context(error),m_file(file),m_line(line){}
    std::string get_name()const;
    std::string get_attribute(const std::string &)const;
    std::string get_description()const;
private:
    std::string m_file;
    int m_line;
};

} //namespace cubescript

#endif
