/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/eval_stream.hpp"
#include "fungu/script/expression.hpp"
#include "fungu/script/error.hpp"

namespace fungu{
namespace script{

eval_stream::eval_stream(env::frame * frame)
: m_expression(new base_expression),
  m_scope(frame),
  m_parsing(false),
  m_buffer_use(&m_first_buffer[0]),
  m_dynbuf_size(0)
{
    reset_buffer();
}

eval_stream::~eval_stream()
{
    reset_expression();
    reset_buffer();
}

void eval_stream::feed(const void * data,std::size_t length)
{
    if(is_buffer_overflow(length)) expand_buffer(length);
    
    if(!m_parsing && length) m_parsing=true;
    
    memcpy(m_buffer_write,data,length);
    m_buffer_write+=length;
    
    const char * start_of_read=m_buffer_read;
    bool reset=false;
    
    try
    {
        if(m_expression->parse(&m_buffer_read,m_buffer_write-1,m_scope)==PARSE_COMPLETE)
        {
            if(m_expression->is_empty_expression() == false)
            {
                if(m_evaluator) m_evaluator(m_expression,m_scope);
                else m_expression->eval(m_scope);
            }
            reset = true;
        }
    }
    catch(...)
    {
        reset_expression();
        reset_buffer();
        throw;
    }
    
    if(reset)
    {
        std::size_t parsed_size=m_buffer_read - start_of_read;
        
        reset_expression();
        reset_buffer();
        
        if(parsed_size < length && !m_scope->has_expired()) 
            feed(((const char *)data)+parsed_size,length-parsed_size);
    }
}

bool eval_stream::is_parsing_expression()const
{
    return m_parsing;
}

bool eval_stream::using_dynamic_buffer()const
{
    return m_buffer_use!=&m_first_buffer[0];
}

bool eval_stream::is_buffer_overflow(std::size_t length)const
{
    if(using_dynamic_buffer())
        return (m_buffer_write + length) > m_buffer_use + (m_dynbuf_size - 1);
    else return (m_buffer_write + length) > &m_first_buffer[InitialBufSize-1];
}

void eval_stream::reset_buffer()
{
    if(using_dynamic_buffer())
    {
        free(m_buffer_use);
        m_dynbuf_size=0;
    }
    
    m_buffer_use=&m_first_buffer[0];
    m_buffer_read=m_buffer_use;
    m_buffer_write=m_buffer_use;
}

void eval_stream::expand_buffer(std::size_t length)
{
    bool migrate_first_buffer=false;
    
    std::size_t read_offset=m_buffer_read - m_buffer_use;
    std::size_t write_offset=m_buffer_write - m_buffer_use;
    
    if(!using_dynamic_buffer())
    {
        m_buffer_use=NULL;
        migrate_first_buffer=true;
    }
    
    m_dynbuf_size += length+InitialBufSize;
    m_buffer_use=(char *)realloc(m_buffer_use,m_dynbuf_size);
    
    if(migrate_first_buffer) memcpy(m_buffer_use,&m_first_buffer[0],InitialBufSize);
    
    m_buffer_read = m_buffer_use + read_offset;
    m_buffer_write = m_buffer_use + write_offset;
    
    // The existing expression object contains dangling pointers to the old
    // buffer allocation; we must re-parse everything again with a new
    // expression object.
    reset_expression();
    const char * reread=m_buffer_use;
    m_expression->parse(&reread,m_buffer_read-1,m_scope);
    assert(reread == m_buffer_read );
}

void eval_stream::reset_expression()
{
    delete m_expression;
    m_expression = new base_expression;
    m_parsing = false;
}

void eval_stream::reset()
{
    reset_expression();
    reset_buffer();
}

} //namespace script
} //namespace fungu
