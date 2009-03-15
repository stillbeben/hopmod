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
class quote:public construct
{
public:
    quote();
    parse_state parse(source_iterator * first,source_iterator last,env::frame *);
    result_type eval(env::frame *);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    std::string m_string;
    bool m_escaped;
};

#endif
