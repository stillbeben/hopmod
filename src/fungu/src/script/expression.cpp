/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "fungu/script/pch.hpp"
#endif

#include "fungu/script/expression.hpp"

#include <iostream>

namespace fungu{
namespace script{

bool expression::word_exit_terminals::is_member(const_string::value_type c)
{
    switch(c)
    {
        case '\"': case '\'': case ')': case '[': case ']': case ' ':
        case '\r': case '\n': case '\t': case ';': case '{' :case '}': 
        case '\0':
            return true;
        default: return false;
    }
}

expression::expression()
 :m_parsing(NULL),
  m_first_construct(NULL),
  m_operation_symbol(NULL),
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
    if(!m_source_ctx)
    {
        m_source_ctx = frame->get_env()->get_source_context() ? frame->get_env()->get_source_context()->clone() : NULL;
        m_line = m_source_ctx->get_line_number();
    }
    
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
            throw new error_trace(
                key, const_string(), frame->get_env()->get_source_context()->clone());
        }
    }
    
    if(*first > last) return PARSE_PARSING;
    
    if(is_terminator(**first))
    {
        ++(*first);
        
        if(!is_empty_expression())
        {
            if(is_alias_assignment(frame)) translate_alias_assignment(frame);
            
            fill_arguments_container(frame);
            
            if(m_first_construct->is_string_constant())
                m_operation_symbol = frame->get_env()->lookup_symbol(m_first_construct->eval(frame).to_string());
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
        default:   m_parsing = new expr_word;                        break;
    }
    
    if(*first > last) return PARSE_PARSING;
    
    return parse(first,last,frame);
}

result_type expression::eval(env::frame * frame)
{
    env * environment = frame->get_env();
    env::object * opobj;
    
    const construct * subject_arg = m_first_construct;
    
    const source_context * prev_ctx = environment->get_source_context();
    environment->set_source_context(m_source_ctx);
    
    int pre_argc = environment->get_arg_count();
    
    #define EXPRESSION_EVAL_CLEANUP \
        reset_placeholders(); \
        environment->set_source_context(prev_ctx); \
        while(environment->get_arg_count() > pre_argc) environment->pop_arg();
    
    try
    {
        result_type arg1_result_type = m_first_construct->eval(frame);
        
        if(m_operation_symbol) opobj = m_operation_symbol->lookup_object(frame);
        else
        {
            m_operation_symbol = frame->get_env()->lookup_symbol(arg1_result_type.to_string());
            if(m_operation_symbol) opobj = m_operation_symbol->lookup_object(frame);
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
        std::vector<unsigned char>::const_iterator argIt = m_placeholders.begin();
        
        while(current)
        {
            subject_arg = current;
            
            environment->push_arg(current->eval(frame));
            
            m_arguments[*(argIt++)] = environment->top_arg();
            
            current = current->get_next_sibling();
        }
        
        subject_arg = m_first_construct;
        
        arguments_container args(m_arguments);
        result_type result = opobj->call(args, frame);
        
        EXPRESSION_EVAL_CLEANUP;
        
        return result;
    }
    catch(const error & key)
    {
        EXPRESSION_EVAL_CLEANUP;
        
        throw new error_trace(
            key, subject_arg->form_source(), get_source_context());
    }
    catch(error_trace * head_info)
    {
        EXPRESSION_EVAL_CLEANUP;
        
        throw new error_trace(
            head_info, subject_arg->form_source(), get_source_context());
    }
    
    #undef EXPRESSION_EVAL_CLEANUP
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
    
bool expression::is_alias_assignment(env::frame * frame)const
{
    construct * arg1 = m_first_construct;
    if(!arg1) return false;
    construct * op = arg1->get_next_sibling();
    if(!op || !op->is_string_constant()) return false;
    construct * arg2 = op->get_next_sibling();
    if(!arg2 || arg2->get_next_sibling()) return false;
    return op->eval(frame).to_string() == const_string(FUNGU_LITERAL_STRING("="));
}

void expression::translate_alias_assignment(env::frame * frame)
{
    construct * arg1 = m_first_construct;
    construct * op = arg1->get_next_sibling();
    construct * arg2 = op->get_next_sibling();
    
    m_first_construct = new expr_word(const_string(FUNGU_LITERAL_STRING("alias")));
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
            m_arguments.push_back(any());
            m_placeholders.push_back(m_arguments.size()-1);
            
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

void expression::reset_placeholders()
{
    for(std::vector<unsigned char>::const_iterator it = m_placeholders.begin(); 
        it != m_placeholders.end(); it++) m_arguments[*it] = any();
}

parse_state base_expression::parse(source_iterator * first,source_iterator last,env::frame * frame)
{
    parse_state state = expression::parse(first,last,frame);
    if(state == PARSE_COMPLETE && *((*first)-1) == ')')
        throw new error_trace(
            error(UNEXPECTED_SYMBOL,boost::make_tuple(')')),
            const_string(), get_source_context());
    return state;
}

} //namespace script
} //namespace fungu
