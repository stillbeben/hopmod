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
    std::vector<boost::tuple<source_iterator,source_iterator,macro *> > m_macros;
    int m_nested;
};

#endif
