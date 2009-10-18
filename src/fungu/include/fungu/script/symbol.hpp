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
template<typename ExitTerminals>
class symbol:public construct
{
public:
    symbol();
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    any eval(env_frame * frame);
    bool is_string_constant()const;
    std::string form_source()const;
protected:
    void assert_parsed()const;
    const_string get_full_id()const;
    env_object * resolve_symbol(env_frame * frame);
private:
    source_iterator * get_memv_tail();
    int get_memv_size()const;
    const_string get_member(int i)const;
    
    source_iterator m_first;
    source_iterator m_last;
    source_iterator m_members[6];
    env::symbol * m_symbol;
};

#endif
