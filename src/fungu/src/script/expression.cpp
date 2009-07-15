/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

static inline bool is_expr_terminator(const_string::value_type c)
{
    switch(c)
    {
        case ';': case ')': case '\r': case '\n': case '\0': return true;
        default: return false;
    }
}

static inline bool is_numeric(const const_string & str)
{
    const_string::const_iterator i=str.begin();
    if(i<str.end()-1 && (*i=='-' || *i=='+')) ++i;
    for(; i!=str.end(); ++i)
    {
        if(!(*i>='0' && *i<='9')) return false;
    }
    return true;
}

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
    
parse_state expression::parse(source_iterator * first, source_iterator last, env::frame * frame)
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
            source_iterator start_pos = *first;
            
            parse_state substate = m_parsing->parse(first, last, frame);
            if( substate != PARSE_COMPLETE ) return substate;
            
            if( start_pos == *first && !is_expr_terminator(*start_pos)) 
                throw error(UNEXPECTED_SYMBOL, boost::make_tuple(**first));
            
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
    
    if(is_expr_terminator(**first))
    {
        ++(*first);
        
        if(!is_empty_expression())
        {
            if(is_alias_assignment(frame)) translate_alias_assignment(frame);
            
            fill_constarg_vector(frame);
            
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
    
    expression * _this = this;
    BOOST_SCOPE_EXIT( (_this)(environment)(prev_ctx) )
    {
        _this->reset_placeholders();
        environment->set_source_context(prev_ctx);
    } BOOST_SCOPE_EXIT_END

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
        
        std::vector<result_type> vargs(m_placeholders.size());
        std::vector<result_type>::iterator vargIter = vargs.begin();
        
        for(construct * current = m_first_construct->get_next_sibling();
            current; current = current->get_next_sibling(), vargIter++ )
        {
            subject_arg = current;
            *vargIter = current->eval(frame);
        }
        
        std::vector<unsigned char>::const_iterator phIter = m_placeholders.begin();
        
        for(vargIter = vargs.begin(); vargIter != vargs.end() ; ++vargIter, ++phIter)
        {
            m_arguments[*phIter] = *vargIter;
        }
        
        subject_arg = m_first_construct;
        
        callargs args(m_arguments);
        result_type result = opobj->call(args, frame);
        
        return result;
    }
    catch(const error & key)
    {
        throw new error_trace(
            key, subject_arg->form_source(), get_source_context());
    }
    catch(error_trace * head_info)
    {
        throw new error_trace(
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
    
void expression::fill_constarg_vector(env::frame * frame)
{
    assert(m_first_construct);
    
    construct * last_arg = m_first_construct;
    construct * arg = m_first_construct->get_next_sibling();
    
    while(arg)
    {
        if(arg->is_string_constant())
        {
            result_type argval = arg->eval(frame);
            
            if(argval.get_type() == typeid(const_string))
            {
                const_string argstrval = any_cast<const_string>(argval);
                if(argstrval.length() && is_numeric(argstrval)) argval = lexical_cast<int>(argstrval);
            }
            
            m_arguments.push_back(argval);
            
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
        it != m_placeholders.end(); it++)
    {
        m_arguments[*it] = any();
    }
}

parse_state base_expression::parse(source_iterator * first, source_iterator last, env::frame * frame)
{
    parse_state state = expression::parse(first,last,frame);
    
    if(state == PARSE_COMPLETE && *((*first)-1) == ')')
    {
        throw new error_trace(
            error(UNEXPECTED_SYMBOL, boost::make_tuple(')')),
            const_string(),
            get_source_context() );
    }
    
    return state;
}

} //namespace script
} //namespace fungu
