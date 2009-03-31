/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifdef FUNGU_CUBESCRIPT_EXPRESSION_NESTED_CLASS

class macro:public construct
{
public:
    macro();
    ~macro();
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame);
    result_type eval(env::frame * frame);
    int get_evaluation_level()const;
    bool is_string_constant()const;
    std::string form_source()const;
private:
    int m_escape;
    construct * m_con;
};

#endif
