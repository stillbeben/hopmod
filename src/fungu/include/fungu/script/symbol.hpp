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
class symbol:public construct
{
public:
    symbol()
     : m_first(const_string::null_const_iterator()),
       m_symbol(NULL)
    {
        m_members[0] = NULL;
        m_members[1] = NULL;
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env::frame * frame)
    {
        if(m_first == const_string::null_const_iterator())
        {
            if(**first == '.' || ExitTerminals::is_member(**first)) 
                throw error(UNEXPECTED_SYMBOL,boost::make_tuple(**first));
            
            m_first = *first;
            m_members[0] = m_first;
        }
        
        for(; *first <= last; ++(*first) )
        {
            if(ExitTerminals::is_member(**first))
            {
                if(*m_last == '.') throw error(UNEXPECTED_SYMBOL,boost::make_tuple('.'));
                m_symbol = frame->get_env()->lookup_symbol(get_member(0));
                return PARSE_COMPLETE;
            }
            else if(**first == '.') *get_memv_tail() = (*first) + 1;
            
            m_last = (*first);
        }
        
        return PARSE_PARSING;
    }
    
    result_type eval(env::frame * frame)
    {
        return resolve_symbol(frame)->value();
    }
    
    bool is_string_constant()const
    {
        return false;
    }
    
    std::string form_source()const
    {
        assert_parsed();
        return (std::string(1,'$') + get_full_id().copy());
    }
protected:
    void assert_parsed()const
    {
        assert(m_first != const_string::null_const_iterator());
    }
    
    const_string get_full_id()const
    {
        return const_string(m_first,m_last);
    }
    
    env::object * resolve_symbol(env::frame * frame)
    {
        assert_parsed();
        
        const_string id = get_member(0);
        
        m_symbol = (!m_symbol ? frame->get_env()->lookup_symbol(id) : m_symbol);
        if(!m_symbol) throw error(UNKNOWN_SYMBOL,boost::make_tuple(id.copy()));
        
        env::object * obj = m_symbol->lookup_object(frame);
        if(!obj) throw error(UNKNOWN_SYMBOL, boost::make_tuple(id.copy()));
        
        int members = get_memv_size();
        for(int i = 1; i < members; i++)
        {
            obj = obj->lookup_member(get_member(i));
            if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(get_member(i).copy()));
        }
        
        return obj;
    }
private:
    source_iterator * get_memv_tail()
    {
        int size = sizeof(m_members) / sizeof(source_iterator);
        for(int i = 0; i < size; i++) 
            if(!m_members[i])
            {
                if(i+1 < size) m_members[i+1] = NULL;
                return &m_members[i];
            }
        
        throw error(MEMBER_ACCESS_CHAIN_LIMIT,boost::make_tuple(size));
    }
    
    int get_memv_size()const
    {
        int count = 0;
        int size = sizeof(m_members) / sizeof(source_iterator);
        for(int i = 0; i < size; i++) if(m_members[i]) count++; else break;
        return count;
    }
    
    const_string get_member(int i)const
    {
        assert(i < get_memv_size());
        int size = sizeof(m_members) / sizeof(source_iterator);
        return const_string(m_members[i],(i+1 == size-1 || !m_members[i+1] ? m_last : m_members[i+1] - 2));
    }
    
    source_iterator m_first;
    source_iterator m_last;
    source_iterator m_members[6];
    env::symbol * m_symbol;
};

#endif
