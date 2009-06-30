/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

template<typename Terms>
expression::symbol<Terms>::symbol()
 : m_first(const_string::null_const_iterator()),
   m_symbol(NULL)
{
    m_members[0] = NULL;
    m_members[1] = NULL;
}

template<typename Terms>
parse_state expression::symbol<Terms>::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    if(m_first == const_string::null_const_iterator())
    {
        if(**first == '.' || Terms::is_member(**first)) 
            throw error(UNEXPECTED_SYMBOL,boost::make_tuple(**first));
        
        m_first = *first;
        m_members[0] = m_first;
    }
    
    for(; *first <= last; ++(*first) )
    {
        if(Terms::is_member(**first))
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

template<typename Terms>
result_type expression::symbol<Terms>::eval(env::frame * frame)
{
    return resolve_symbol(frame)->value();
}

template<typename Terms>
bool expression::symbol<Terms>::is_string_constant()const
{
    return false;
}

template<typename Terms>
std::string expression::symbol<Terms>::form_source()const
{
    assert_parsed();
    return (std::string(1,'$') + get_full_id().copy());
}

template<typename Terms>
void expression::symbol<Terms>::assert_parsed()const
{
    assert(m_first != const_string::null_const_iterator());
}

template<typename Terms>
const_string expression::symbol<Terms>::get_full_id()const
{
    return const_string(m_first,m_last);
}

template<typename Terms>
env::object * expression::symbol<Terms>::resolve_symbol(env::frame * frame)
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

template<typename Terms>
construct::source_iterator * expression::symbol<Terms>::get_memv_tail()
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

template<typename Terms>
int expression::symbol<Terms>::get_memv_size()const
{
    int count = 0;
    int size = sizeof(m_members) / sizeof(source_iterator);
    for(int i = 0; i < size; i++) if(m_members[i]) count++; else break;
    return count;
}

template<typename Terms>
const_string expression::symbol<Terms>::get_member(int i)const
{
    assert(i < get_memv_size());
    int size = sizeof(m_members) / sizeof(source_iterator);
    return const_string(m_members[i],(i+1 == size-1 || !m_members[i+1] ? m_last : m_members[i+1] - 2));
}

template class expression::symbol<expression::word_exit_terminals>;
template class expression::symbol<array_word_exit_terminals>;

} //namespace script
} //namespace fungu
