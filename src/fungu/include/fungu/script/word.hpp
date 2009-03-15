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
    word()
     :m_first(const_string::null_const_iterator())
    {
        
    }
    
    word(const_string w)
     : m_first(w.begin()),
       m_last(w.end()-1)
    {
        
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env::frame *)
    {
        if(m_first == const_string::null_const_iterator())
            m_first = *first;
        
        for(; *first <= last; ++(*first) )
        {
            if(ExitTerminals::is_member(**first)) return PARSE_COMPLETE;
            m_last = (*first); // the reason for assigning m_last on each
            //iteration, instead of assigning once when parsing is complete, is
            //to allow exist terminal symbols to be stored outside of the
            //source buffer.
        }
        
        return PARSE_PARSING;
    }
    
    result_type eval(env::frame * frame)
    {
        assert(m_first != const_string::null_const_iterator());
        return const_string(m_first,m_last);
    }
    
    bool is_string_constant()const
    {
        return true;
    }
    
    std::string form_source()const
    {
        assert(m_first != const_string::null_const_iterator());
        return const_string(m_first,m_last).copy();
    }
private:
    source_iterator m_first;
    source_iterator m_last;
};

#endif
