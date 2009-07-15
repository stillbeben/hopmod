/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

expression::comment::comment()
 :m_first(const_string::null_const_iterator())
{
    
}

parse_state expression::comment::parse(source_iterator * first,source_iterator last,env::frame *)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    for(; *first <= last; ++(*first) )
        if(is_terminator(**first))
        {
            m_last = (*first) -1;
            return PARSE_COMPLETE;
        }
    
    return PARSE_PARSING;
}

result_type expression::comment::eval(env::frame *)
{
    return const_string();
}

bool expression::comment::is_eval_supported()const
{
    return false;
}

bool expression::comment::is_string_constant()const
{
    return true;
}

std::string expression::comment::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    const_string source(m_first,m_last);
    return std::string(2,'/') + std::string(source.begin(),source.end());
}

bool expression::comment::is_terminator(const_string::value_type c)
{
    return c == '\r' || c=='\n';
}

} //namespace script
} //namespace fungu
