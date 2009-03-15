/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS

#include "macro.hpp"

/**
    
*/
class block:public construct
{
public:
    block();
    ~block();
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame);
    result_type eval(env::frame *);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    source_iterator m_first;
    source_iterator m_last;
    
    int m_nested;
    
    /*
        If the block contains any macros that are ready for evaluation then the
        block maintains its own source string.
    */
    bool m_copysrc;
    source_iterator m_start_of_macro;
    std::string m_content;
    
    macro * m_parsing_macro;
};

#endif
