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
expression::word<Terms>::word()
 :m_first(const_string::null_const_iterator())
{
    
}

template<typename Terms>
expression::word<Terms>::word(const_string w)
 : m_first(w.begin()),
   m_last(w.end()-1)
{
    
}

template<typename Terms>
parse_state expression::word<Terms>::parse(source_iterator * first,source_iterator last,env::frame *)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    for(; *first <= last; ++(*first) )
    {
        if(Terms::is_member(**first)) return PARSE_COMPLETE;
        m_last = (*first); // the reason for assigning m_last on each
        //iteration, instead of assigning once when parsing is complete, is
        //to allow exist terminal symbols to be stored outside of the
        //source buffer.
    }
    
    return PARSE_PARSING;
}

template<typename Terms>
result_type expression::word<Terms>::eval(env::frame * frame)
{
    assert(m_first != const_string::null_const_iterator());
    return const_string(m_first,m_last);
}

template<typename Terms>
bool expression::word<Terms>::is_string_constant()const
{
    return true;
}

template<typename Terms>
std::string expression::word<Terms>::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    return const_string(m_first,m_last).copy();
}

template class expression::word<expression::word_exit_terminals>;
template class expression::word<array_word_exit_terminals>;

} //namespace script
} //namespace fungu
