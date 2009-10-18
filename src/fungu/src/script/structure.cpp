/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "fungu/script/pch.hpp"
#endif

#include "fungu/script/expression.hpp"

namespace fungu{
namespace script{

expression::structure::structure()
 :m_first(const_string::null_const_iterator()),
  m_nested(0)
{
    
}

parse_state expression::structure::parse(source_iterator * first,source_iterator last,env_frame * frame)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    for(; *first <= last; ++(*first))
    {
        switch(**first)
        {
            case '{':
                m_nested++;
                break;
            case '}':
                if(--m_nested == 0)
                {
                    m_last = *first;
                    ++(*first);
                    if(!parse_json()) throw error(EXTERNAL_PARSE_ERROR);
                    return PARSE_COMPLETE;
                }
                break;
            default:;
        }
    }
    
    return PARSE_PARSING;
}

any expression::structure::eval(env_frame *)
{
    return m_object;
}

bool expression::structure::is_string_constant()const
{
    return true;
}

std::string expression::structure::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    return const_string(m_first,m_last).copy();
}

bool expression::structure::parse_json()
{
    json::variant v = ::json::parse<const char *,any_traits>(m_first, m_last+1);
    if(v->get_type() != typeid(json::object)) return false;
    m_object = boost::shared_ptr<json::object>(new json::object);
    *m_object = any_cast<json::object>(*v);
    return true;
}

} //namespace script
} //namespace fungu
