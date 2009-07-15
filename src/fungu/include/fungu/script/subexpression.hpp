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
class subexpression:public construct
{
public:
    subexpression();
    ~subexpression();
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame);
    result_type eval(env::frame * frame);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    expression * m_parsing;
    expression * m_first_expr;
};

#endif
