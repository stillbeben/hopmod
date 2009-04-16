/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_PARSE_ARRAY_HPP
#define FUNGU_SCRIPT_PARSE_ARRAY_HPP

#include "expression.hpp"
#include "lexical_cast.hpp"

namespace fungu{
namespace script{

struct array_word_exit_terminals
{
    static bool is_member(const_string::value_type c)
    {
        switch(c)
        {
            case '\"': case '\'': case ')': case '[': case ']': case ' ':
            case '\r': case '\n': case '\t': case ';': case ',': case '\0':
                return true;
            default: return false;
        }
    }
};

template<typename ForwardContainer,bool throw_exception>
inline bool parse_array(const_string string, env::frame * frame, ForwardContainer & container)
{
    construct * parsing = NULL;
    const_string::const_iterator current = string.begin();
    
    for(; current < string.end(); ++current)
    {
        if( *current == ' ' || *current == '\t' || *current=='\r' ||
            *current == '\n') continue;
        
        char termc;
        const_string::const_iterator start = current;
        
        switch(*current)
        {
            case '[':
                parsing = new expression::block; 
                current++;
                termc = ']';
                break;
            case '\"': 
                parsing = new expression::quote;
                current++;
                termc = '\"';
                break;
            case '/': case '#':
                parsing = new expression::comment;
                current++;
                termc = '\n';
                break;
            case '$':
                if(throw_exception) throw error(UNEXPECTED_SYMBOL,boost::make_tuple('$'));
                else return false;
            default:
                parsing = new expression::word<array_word_exit_terminals>;
                termc = 'w';
        }
        
        if(parsing->parse(&current,string.end()-1, frame) != PARSE_COMPLETE)
        {
            //acceptable cases
            if( (termc == 'w' || termc=='\n') && current == string.end()) 
            {
                current--;
                start--;
            }
            else
            {
                if(throw_exception) throw error(EXPECTED_SYMBOL,boost::make_tuple(termc));
                else return false;
            }
        }
        
        if(current == start) throw error(UNEXPECTED_SYMBOL,boost::make_tuple(*current));
        
        if(parsing->is_eval_supported())
        {
            try
            {
                container.push_back(lexical_cast<typename ForwardContainer::value_type>(parsing->eval(frame).to_string().copy()));
            }
            catch(error err)
            {
                if(throw_exception) throw;
                delete parsing;
                return false;
            }
        }
        
        delete parsing;
    }
    
    return true;
}

} //namespace script
} //namespace fungu

#endif
