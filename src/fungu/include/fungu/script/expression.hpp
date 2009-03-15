/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_EXPRESSION_HPP
#define FUNGU_SCRIPT_EXPRESSION_HPP

#include "construct.hpp"
#include "error.hpp"
#include "typed_literal.hpp"

#include <limits>
#include <boost/spirit/include/classic_core.hpp> //used for int parsing

namespace fungu{
namespace script{

/**
    
*/
class expression:public construct
{
public:
    struct word_exit_terminals
    {
        static bool is_member(const_string::value_type c)
        {
            switch(c)
            {
                case '\"': case '\'': case ')': case '[': case ']': case ' ':
                case '\r': case '\n': case '\t': case ';': case '{' :case '}': 
                case '\0':
                    return true;
                default: return false;
            }
        }
    };
    
    #define FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS
    #include "subexpression.hpp"
    #include "word.hpp"
    #include "symbol.hpp"
    #include "reference.hpp"
    #include "quote.hpp"
    #include "block.hpp"
    #include "comment.hpp"
    #include "structure.hpp"
    #undef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS
    
    typedef word<word_exit_terminals> expr_word;
    typedef symbol<word_exit_terminals> expr_symbol;
    typedef reference<word_exit_terminals> expr_reference;
    
    expression();
    ~expression();
    
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame);
    
    result_type eval(env::frame * frame);
    
    bool is_string_constant()const;
    
    /**
        @brief Return formed expression.
        
        Reconstructs expression's source code.
    */
    std::string form_source()const;
    
    /**
        @brief Check for empty empression.
        
        Returns true if the expression object contains no arguments (including
        the operation argument).
        
        @internal The method is used in alias evaluation, for ingoring the 
                  evaluation result of empty expressions when setting the
                  implicit alias result.
    */
    bool is_empty_expression()const;
private:
    static inline bool is_terminator(const_string::value_type c);
    static inline bool is_numeric(const const_string & str);
    
    void add_child_construct(construct * child);
    
    bool is_infix_expression(env::frame * frame)const;
    void translate_infix_expression(env::frame * frame);
    
    void fill_arguments_container(env::frame * frame);
protected:
    source_context * get_source_context()const;
private:
    construct * m_parsing;
    construct * m_first_construct;
    
    env::object::apply_arguments m_arguments;
    
    env::symbol_handle m_operation_symbol;
    
    // information for debugging
    unsigned short m_line;
    const source_context * m_source_ctx;
};

class base_expression:public expression
{
public:
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame);
};

} //namespace script
} //namespace fungu

#endif
