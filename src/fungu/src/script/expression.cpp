/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/expression.hpp"

#include <limits>
#include <boost/spirit/include/classic_core.hpp> //used for int parsing

namespace fungu{
namespace script{

expression::expression()
 :m_parsing(NULL),
  m_first_construct(NULL),
  m_operation_symbol(env::invalid_symbol_handle()),
  m_line(0),
  m_source_ctx(NULL)
{
    
}

expression::~expression()
{
    delete m_parsing;
    delete m_first_construct;
}
    
parse_state expression::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    if(m_parsing)
    {
        try
        {
            source_iterator last_pos = *first;
            parse_state substate=m_parsing->parse(first,last,frame);
            if( substate != PARSE_COMPLETE ) return substate;
            
            if( last_pos == *first && !is_terminator(*last_pos)) 
                throw error(UNEXPECTED_SYMBOL,boost::make_tuple(**first));
            
            if(m_parsing->is_eval_supported()) add_child_construct(m_parsing);
            else delete m_parsing;
            
            m_parsing = NULL;
        }
        catch(const error & key)
        {
            throw new error_info(
                key, const_string(), frame->get_env()->get_source_context()->clone());
        }
    }
    
    if(*first > last) return PARSE_PARSING;
    
    if(is_terminator(**first))
    {
        if(frame->get_env()->get_source_context())
        {
            m_source_ctx = frame->get_env()->get_source_context();
            m_line = m_source_ctx->get_line_number();
        }
        
        ++(*first);
        
        if(!is_empty_expression())
        {
            if(is_infix_expression(frame)) translate_infix_expression(frame);
            
            fill_arguments_container(frame);
            
            if(m_first_construct->is_string_constant())
                m_operation_symbol = env::lookup_symbol(m_first_construct->eval(frame).to_string());
        }
        
        return PARSE_COMPLETE;
    }
    
    switch(**first)
    {
        case '\"': m_parsing = new quote;                ++(*first); break;
        case '[':  m_parsing = new block;                ++(*first); break;
        case '(':  m_parsing = new subexpression;        ++(*first); break;
        case '/':
        case '#':  m_parsing = new comment;              ++(*first); break;
        case ' ':
        case '\t':                                       ++(*first); break;
        case '$':  m_parsing = new expr_symbol;          ++(*first); break;
        case '&':  m_parsing = new expr_reference;       ++(*first); break;
        //case '{': m_parsing = new structure;                         break;
        default:   m_parsing = new expr_word;                        break;
    }
    
    if(*first > last) return PARSE_PARSING;
    
    return parse(first,last,frame);
}

result_type expression::eval(env::frame * frame)
{
    env::object * opobj;
    
    const construct * subject_arg = m_first_construct;
    
    try
    {
        result_type arg1_result_type = m_first_construct->eval(frame);
        
        if(env::is_valid_symbol_handle(m_operation_symbol))
            opobj = frame->lookup_object(m_operation_symbol);
        else
        {
            if((m_operation_symbol = env::lookup_symbol(arg1_result_type.to_string())))
                opobj = frame->lookup_object(m_operation_symbol);
            else
            {
                if(arg1_result_type.get_type() == typeid(env::object::shared_ptr))
                    opobj = any_cast<env::object::shared_ptr>(arg1_result_type).get();
                else opobj = NULL;
            }
        }
        
        if(!opobj)
        {
            const_string opsym = arg1_result_type.to_string();
            if(is_numeric(opsym)) return opsym;
            else throw error(UNKNOWN_SYMBOL,boost::make_tuple(opsym.copy()));
        }
        
        construct * current = m_first_construct->get_next_sibling();
        while(current)
        {
            subject_arg = current;
            m_arguments.assign_next_placeholder(current->eval(frame));
            current = current->get_next_sibling();
        }
        
        subject_arg = m_first_construct;
        
        result_type result = opobj->apply(m_arguments,frame);
        m_arguments.reset();
        return result;
    }
    catch(const error & key)
    {
        m_arguments.reset();
        throw new error_info(
            key, subject_arg->form_source(), get_source_context());
    }
    catch(error_info * head_info)
    {
        m_arguments.reset();
        throw new error_info(
            head_info, subject_arg->form_source(), get_source_context());
    }
}
    
bool expression::is_string_constant()const
{
    return false;
}

std::string expression::form_source()const
{
    std::string source;
    bool multiple=false;
    
    construct * current = m_first_construct;
    while(current)
    {
        if(multiple) source+=" ";
        else multiple=true;
        source+=current->form_source();
        
        current = current->get_next_sibling();
    }
    
    source+="\n";
    return source;
}

bool expression::is_empty_expression()const
{
    return !m_first_construct;
}

bool expression::is_terminator(const_string::value_type c)
{
    static const_string termset(FUNGU_LITERAL_STRING(";)\r\n\0"));
    return construct::is_terminator(c,termset);
}

inline bool expression::is_numeric(const const_string & str)
{
    const_string::const_iterator i=str.begin();
    if(i<str.end()-1 && (*i=='-' || *i=='+')) ++i;
    for(; i!=str.end(); ++i)
        if(!(*i>='0' && *i<='9') && !(i>str.begin() && *i=='.') ) 
            return false;
    return true;
}
    
void expression::add_child_construct(construct * child)
{
    if(!m_first_construct) m_first_construct = child;
    else m_first_construct->get_tail_sibling()->set_next_sibling(child);
}
    
bool expression::is_infix_expression(env::frame * frame)const
{
    construct * arg1 = m_first_construct;
    if(!arg1) return false;
    construct * op = arg1->get_next_sibling();
    if(!op || !op->is_string_constant()) return false;
    construct * arg2 = op->get_next_sibling();
    if(!arg2 || arg2->get_next_sibling()) return false;
    return frame->get_env()->is_infix_operator(op->eval(frame).to_string());
}
    
void expression::translate_infix_expression(env::frame * frame)
{
    construct * arg1 = m_first_construct;
    construct * op = arg1->get_next_sibling();
    construct * arg2 = op->get_next_sibling();
    
    m_first_construct = new expr_word(frame->get_env()->get_infix_operator_mapping(op->eval(frame).to_string()));
    m_first_construct->set_next_sibling(arg1);
    arg1->set_next_sibling(arg2);
    
    op->set_next_sibling(NULL);
    delete op;
}
    
void expression::fill_arguments_container(env::frame * frame)
{
    assert(m_first_construct);
    
    construct * last_arg = m_first_construct;
    construct * arg = m_first_construct->get_next_sibling();
    
    while(arg)
    {
        if(arg->is_string_constant())
        {
            m_arguments.push_back(arg->eval(frame));
            
            construct * next = arg->get_next_sibling();
            
            arg->set_next_sibling(NULL);
            delete arg;
            
            last_arg->set_next_sibling(next);
            arg = next;
        }
        else
        {
            m_arguments.push_back_placeholder();
            last_arg = arg;
            arg = arg->get_next_sibling();
        }
    }
}

source_context * expression::get_source_context()const
{
    if(!m_source_ctx) return NULL;
    source_context * ctx = m_source_ctx->clone();
    ctx->set_line_number(m_line);
    return ctx;
}

parse_state base_expression::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    parse_state state = expression::parse(first,last,frame);
    if(state == PARSE_COMPLETE && *((*first)-1) == ')')
        throw new error_info(
            error(UNEXPECTED_SYMBOL,boost::make_tuple(')')),
            const_string(), get_source_context());
    return state;
}

} //namespace script
} //namespace fungu
