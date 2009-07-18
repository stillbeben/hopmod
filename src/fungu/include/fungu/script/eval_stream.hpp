/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_EVAL_STREAM_HPP
#define FUNGU_SCRIPT_EVAL_STREAM_HPP

#include "env.hpp"
#include "expression.hpp"

namespace fungu{
namespace script{

/**
    
*/
class eval_stream
{
    static const std::size_t InitialBufSize = 512; //enough space to store the code for a single base expression
public:
    typedef void (* evaluation_function)(expression *, env::frame *, void *);
    eval_stream(env::frame *, evaluation_function evaluator = NULL, void * evaluator_closure = NULL);
    ~eval_stream();
    void feed(const void * data, std::size_t bytesLength);
    bool is_parsing_expression()const;
    void reset();
private:
    bool would_overflow_buffer(std::size_t)const;
    void expand_buffer(std::size_t);
    bool using_dynamic_buffer()const;
    void reset_buffer();
    void reset_expression();
    
    evaluation_function m_evaluator;
    void * m_evaluator_closure;
    
    base_expression m_expression;
    env::frame * m_frame;
    bool m_parsing;
    
    char * m_buffer_use;
    const char * m_buffer_read;
    char * m_buffer_write;
    
    char m_first_buffer[InitialBufSize+1];
    std::size_t m_dynbuf_size;
};

} //namespace script
} //namespace fungu

#endif
