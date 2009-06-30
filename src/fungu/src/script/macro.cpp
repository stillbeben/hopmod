/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

expression::macro::macro()
 :m_escape(1),m_con(NULL)
{
    
}

expression::macro::~macro()
{
    if(m_con) delete m_con;
}

parse_state expression::macro::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    if(m_con) return m_con->parse(first,last,frame);
    
    switch(**first)
    {
        case '@': m_escape++;                ++(*first); break;
        case '(': m_con = new subexpression; ++(*first); break;
        default: m_con = new expr_symbol;
    }
    
    return parse(first,last,frame);
}

result_type expression::macro::eval(env::frame * frame)
{
    assert(m_con);
    return m_con->eval(frame);
}

int expression::macro::get_evaluation_level()const
{
    return m_escape;
}

bool expression::macro::is_string_constant()const
{
    return m_con->is_string_constant();
}

std::string expression::macro::form_source()const
{
    return std::string(m_escape,'@') + m_con->form_source();
}

} //namespace script
} //namespace fungu
