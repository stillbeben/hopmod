/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS

/**
    
*/
class comment:public construct
{
public:
    comment();
    parse_state parse(source_iterator * first,source_iterator last,env::frame *);
    result_type eval(env::frame *);
    bool is_eval_supported()const;
    bool is_string_constant()const;
    std::string form_source()const;
private:
    static inline bool is_terminator(const_string::value_type c);
private:
    source_iterator m_first;
    source_iterator m_last;
};

#endif
