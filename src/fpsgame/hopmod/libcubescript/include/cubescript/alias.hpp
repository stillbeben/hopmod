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
#ifndef LIBCUBESCRIPT_ALIAS_HPP
#define LIBCUBESCRIPT_ALIAS_HPP

#include <stack>
#include "predefs.hpp"
#include "symbol.hpp"

namespace cubescript{

/*!
    @brief The symbol class for hosting aliases.
    
    An alias in the CubeScript programming language is a named string which can
    be executed or its stored value returned; an alias can serve as a variable
    or function for the script writer. Aliases are typically created and exist
    at runtime only; the alias assignment function registers newly created
    aliases to the domain passed from the expression object.
*/
class alias:public symbol::persistable
{
public:
    alias();
    
    /*!
        @brief Push value onto alias stack.
        
        The alias object stores a stack of values, when the apply or value
        method is called, the last pushed value is always the value used
        by these two methods.
        
        @note push and pop are accessible runtime functions.
    */
    void push(const std::string &);
    /*!
        @brief Remove latest value from alias stack.
    */
    bool pop();

    /*!
        @brief Call the alias with the given argument list.
        
        Evaluates the stored code block, with the given arguments. The passed
        arguments are accessible, by the alias code block, under symbol names
        using the convention $arg[1 to n], for example $arg1 and $arg2.
    */
    std::string apply(std::list<std::string> & args,domain *);
    /*!
        @brief Get the alias's stored value.
    */
    std::string value()const;
    
    std::string save(const std::string &)const;
    
private:
    void create_sub_alias(domain *,const std::string &,const std::string &);
    void create_reference(domain *,domain *,const std::string &,const std::string &);
    void create_parameter_refs(std::list<std::string> &,domain *);
    void create_my_variable(const std::string &,const std::string &);
    
    void result(std::string);
    
    std::string eval_block(domain *);
    std::stack<std::string> m_blocks;
    std::string m_result;
    bool m_set_result;
    domain * m_local;
};

class alias_function
{
public:
    alias_function(const std::string &,domain *);
    std::string run(std::list<std::string> &);
private:
    alias m_alias;
    domain * m_domain;
};

} //namespace cubescript

#endif
