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
#ifndef LIBCUBESCRIPT_CONSTRUCT_EXPRESSION_HPP
#define LIBCUBESCRIPT_CONSTRUCT_EXPRESSION_HPP

#include "../construct.hpp"
#include "../error.hpp"

namespace cubescript{
namespace cons{
/*!
    The expression construct is the core execution construct of the scripting
    language, an expression represents an apply operation to be performed on a
    symbol object, the meaning of apply is dependant on the symbol object's
    type. A CubeScript programmer writes a script from a sequence of
    expressions; no distinction is made between statements and expressions,
    the engine treats all expressions as one type of thing: an apply operation.
    Apply operations are abstract to the expression evaluator, only the script
    writer knows what apply operation is performed, by the symbol name given.

    An expression consists of elements separated by whitespace
    characters, with the first element being the symbol name and the remaining
    elements being the arguments. An expression is terminated by a newline or
    semicolon character. An element can be a word, string, block or expression.
    When the element constructs have been evaluated, from left to right, the
    apply method will be called on the symbol object. The result value of the
    apply operation is the result of the expression.

    There is an exception to the rule of expected syntax: the expression
    construct can also detect and accept an alias assignment using infix
    notation (i.e. "name = [value]"), for convenient use.
*/
class expression:public construct
{
public:
    expression();
    ~expression();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
    bool is_empty()const;
    bool is_parsing()const;
    std::string get_operation_id()const throw();
private:
    bool parse_sub_construct(std::istream &);
    inline bool in_termset(char)const;
    bool assign_alias_macro(std::list<std::string> &);
    inline bool eval_type(const construct *)const;
    
    construct * m_parsing;
    char m_sub_term;
    std::streampos m_inputpos;
    
    std::list<construct *> m_ops;
};

/*!
    The expression_container construct is used for expressing sub-expressions.
    The syntax for a sub-expression is: "(" <expression> ")".
*/
class expression_container:public construct
{
public:
    expression_container();
    ~expression_container();
    bool parse(std::istream &);
    std::string eval();
    std::string formed()const;
private:
    expression * m_parsing;
    std::list<expression *> m_exps;
};

} //namespace cons

class expr_error_context:public error_context
{
public:
    template<typename T>
     expr_error_context(const T & error,const cons::expression * expr)
      :error_context(error),m_opid(expr->get_operation_id()),m_ctx(expr->formed()){}
    std::string get_name()const;
    std::string get_description()const;
    std::string get_attribute(const std::string &)const;
private:
    std::string m_opid;
    std::string m_ctx;
};

class deref_error_context:public error_context
{
public:
    template<typename T>
    deref_error_context(const T & error,const std::string & id,int level)
     :error_context(error),m_id(id),m_level(level){}
    std::string get_name()const;
    std::string get_description()const;
    std::string get_attribute(const std::string &)const;
private:
    std::string m_id;
    int m_level;
};

} //namespace cubescript

#endif
