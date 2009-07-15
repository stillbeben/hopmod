/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

expression::subexpression::subexpression()
 :m_parsing(NULL),m_first_expr(NULL)
{
    
}

expression::subexpression::~subexpression()
{
    delete m_parsing;
    delete m_first_expr;
}

parse_state expression::subexpression::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    if(!m_parsing) m_parsing = new expression;
    
    parse_state substate = m_parsing->parse(first,last,frame);
    if( substate != PARSE_COMPLETE) return substate;
    
    if(!m_parsing->is_empty_expression()) 
    {
        if(!m_first_expr) m_first_expr = m_parsing;
        else
            m_first_expr->get_tail_sibling()->set_next_sibling(m_parsing);
    }
    
    m_parsing = NULL;
    
    if( *((*first)-1) != ')' )
        return parse(first,last,frame);
    
    return PARSE_COMPLETE;
}

result_type expression::subexpression::eval(env::frame * frame)
{
    result_type result;
    construct * expr = m_first_expr;
    while(expr)
    {
        result = expr->eval(frame);
        expr = expr->get_next_sibling();
    }
    return result;
}

bool expression::subexpression::is_string_constant()const
{
    return false;
}

std::string expression::subexpression::form_source()const
{
    std::string source = "(";
    bool multiple = false;
    construct * expr = m_first_expr;
    
    while(expr)
    {
        if(multiple) source += ";";
        else multiple = true;
        
        std::string exprsrc = expr->form_source();
        exprsrc.erase(exprsrc.end()-1);
        source += exprsrc;
        
        expr = expr->get_next_sibling();
    }
    
    source += ")";
    return source;
}

} //namespace script
} //namespace fungu
