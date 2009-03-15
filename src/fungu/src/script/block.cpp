/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/expression.hpp"

namespace fungu{
namespace script{

expression::block::block()
 :m_first(const_string::null_const_iterator()),
  m_nested(0),
  m_copysrc(false),
  m_parsing_macro(NULL)
{
    
}

expression::block::~block()
{
    if(m_parsing_macro) delete m_parsing_macro;
}

parse_state expression::block::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    if(m_parsing_macro)
    {
        parse_state substate = m_parsing_macro->parse(first,last,frame);
        if(substate != PARSE_COMPLETE) return substate;
        
        if(m_parsing_macro->get_evaluation_level() > m_nested)
        {
            if(!m_copysrc)
            {
                m_content.reserve(64);
                m_content.assign(m_first,m_start_of_macro);
                m_copysrc = true;
            }
            
            m_content += m_parsing_macro->eval(frame).to_string();
        }
        else if(m_copysrc) m_content.append(m_start_of_macro,*first);
        
        delete m_parsing_macro;
        m_parsing_macro = NULL;
    }
    
    for(; *first <= last; ++(*first))
    {
        switch(**first)
        {
            case '[': m_nested++; break;
            case ']':
                if(!m_nested)
                {
                    m_last = (*first)-1;
                    ++(*first);
                    return PARSE_COMPLETE;
                }
                m_nested--;
                break;
            case '@':
                m_start_of_macro = *first;
                ++(*first);
                m_parsing_macro = new macro;
                return parse(first,last,frame);
            default:;
        }
        
        if(m_copysrc) m_content += **first;
    }
    
    return PARSE_PARSING;
}

result_type expression::block::eval(env::frame *)
{
    assert(m_first != const_string::null_const_iterator());
    if(m_copysrc) return const_string(m_content);
    return const_string(m_first,m_last);
}

bool expression::block::is_string_constant()const
{
    return true;
}

std::string expression::block::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    std::string source;
    if(m_copysrc) source=m_content;
    else source+=const_string(m_first,m_last);
    return std::string(1,'[') + source + std::string(1,']');
}

} //namespace script
} //namespace fungu
