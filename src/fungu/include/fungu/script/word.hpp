/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS

/**
    
*/
template<typename ExitTerminals>
class word:public construct
{
public:
    word();    
    word(const_string w);
    parse_state parse(source_iterator * first,source_iterator last,env::frame *);
    result_type eval(env::frame * frame);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    source_iterator m_first;
    source_iterator m_last;
};

#endif
