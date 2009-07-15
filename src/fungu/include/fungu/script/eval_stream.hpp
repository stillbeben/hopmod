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
#include <boost/function.hpp>

namespace fungu{
namespace script{

class expression;
class base_expression;

/**
    
*/
class eval_stream
{
    static const std::size_t InitialBufSize = 4096;
public:
    template<typename EvalFunction>
    eval_stream(env::frame * frame,EvalFunction evaluator)
     :m_evaluator(evaluator),
      m_expression(new base_expression),
      m_scope(frame),
      m_parsing(false),
      m_buffer_use(&m_first_buffer[0]),
      m_dynbuf_size(0)
    {
        reset_buffer();
    }
    eval_stream(env::frame *);
    ~eval_stream();
    void feed(const void * data,std::size_t length);
    bool is_parsing_expression()const;
    void reset();
private:
    bool using_dynamic_buffer()const;
    bool is_buffer_overflow(std::size_t length)const;
    void reset_buffer();
    void expand_buffer(std::size_t length);

    void reset_expression();

    boost::function<void (expression *,env::frame *)> m_evaluator;
    
    expression * m_expression;
    env::frame * m_scope;
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
