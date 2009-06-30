/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

code_block::code_block()
 :m_source_ctx(NULL)
{
    assert(false);
}

code_block::code_block(const_string source,const source_context * src_ctx)
 :m_source(source.copy()),
  m_source_ctx(src_ctx ? src_ctx->clone() : new local_source_context)
{
    
}

code_block::code_block(const code_block & src)
 :m_source(src.m_source),
  m_first_expression(src.m_first_expression),
  m_source_ctx(src.m_source_ctx->clone())
{
    
}

code_block::~code_block()
{
    delete m_source_ctx;
}

code_block & code_block::operator=(const code_block & src)
{
    delete m_source_ctx;
    
    m_source = src.m_source;
    m_first_expression = src.m_first_expression;
    m_source_ctx = src.m_source_ctx->clone();
    
    return *this;
}

code_block code_block::temporary_source(const_string source,const source_context * src_ctx)
{
    code_block tmp(const_string(),NULL);
    tmp.m_source = source;
    tmp.m_source_ctx = src_ctx ? src_ctx->clone() : new local_source_context;
    return tmp;
}

code_block & code_block::compile(env::frame * aScope)
{
    const source_context * last_context = aScope->get_env()->get_source_context();
    aScope->get_env()->set_source_context(m_source_ctx);
    int first_line = m_source_ctx->get_line_number();
    
    expression * current = new base_expression;
    expression * before_current = NULL;
    
    const_string::const_iterator readptr = m_source.begin();
    
    #define COMPILE_CLEANUP \
        aScope->get_env()->set_source_context(last_context); \
        m_source_ctx->set_line_number(first_line); \
        delete current;
    
    while(readptr != m_source.end())
    {
        try
        {
            if(current->parse(&readptr, m_source.end()-1, aScope) != PARSE_COMPLETE)
            {
                const char * lf = "\n";
                current->parse(&lf,lf,aScope);
            }
        }
        catch(error_trace *)
        {
            COMPILE_CLEANUP;
            throw;
        }
        catch(error_exception)
        {
            COMPILE_CLEANUP;
            throw;
        }
        
        if(!current->is_empty_expression())
        {
            if(before_current) before_current->set_next_sibling(current);
            else m_first_expression = boost::shared_ptr<expression>(current);
            before_current = current;
        }
        else delete current;
        
        if(scan_newline(&(--readptr)))
            m_source_ctx->set_line_number(m_source_ctx->get_line_number()+1);
        else readptr++;
        
        current = new base_expression;
    }
    
    COMPILE_CLEANUP;
    
    return *this;
}

bool code_block::should_compile()
{
    return !m_first_expression;
}

void code_block::destroy_compilation()
{
    m_first_expression = boost::shared_ptr<expression>();
}

code_block::iterator code_block::begin()
{
    return iterator(m_first_expression.get());
}

code_block::iterator code_block::end()
{
    return iterator(NULL);
}

result_type code_block::eval_each_expression(env::frame * aScope)
{
    result_type last_result;
    for(construct * e = m_first_expression.get(); 
        e && !aScope->has_expired(); 
        e = e->get_next_sibling() )
    {
        last_result = e->eval(aScope);
    }
    if(!aScope->get_result_value().empty())
        last_result = aScope->get_result_value();
    return last_result;
}

result_type code_block::value()const
{
    return m_source;
}

const source_context * code_block::get_source_context()const
{
    return m_source_ctx;
}

} //namespace script
} //namespace fungu
