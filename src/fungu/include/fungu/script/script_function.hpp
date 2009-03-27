/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SCRIPT_FUNCTION_HPP
#define FUNGU_SCRIPT_SCRIPT_FUNCTION_HPP

#include "arguments_container.hpp"
#include "call_serializer.hpp"
#include "../generic_script_function.hpp"
#include <boost/function.hpp>

namespace fungu{
namespace script{

template<typename Signature>
class script_function:public generic_script_function<Signature, arguments_container, call_serializer, error>
{
public:
    template<typename ErrorHandlerFunction>
    script_function(env::object::shared_ptr object,env::frame * frame,ErrorHandlerFunction error_handler)
     :m_object(object),
      m_frame(frame),
      m_serializer(m_args,frame),
      m_error_handler(error_handler)
    {
        
    }
protected:
    void init(arguments_container ** args,call_serializer ** serializer)
    {
        *args = &m_args;
        *serializer = &m_serializer;
    }
    
    arguments_container::value_type call(arguments_container * args)
    {
        arguments_container::value_type result;
        try
        {
            result = m_object->apply(m_args,m_frame);
        }
        catch(error e)
        {
            result = m_error_handler(create_error_info(e,m_frame->get_env()->get_source_context()));
        }
        catch(error_info * errinfo)
        {
            result = m_error_handler(errinfo);
        }
        m_args.clear();
        m_serializer.clear();
        return result;
    }
    
    arguments_container::value_type error_handler(int arg, error err)
    {
        return m_error_handler(create_error_info(err,NULL));
    }
private:
    error_info * create_error_info(error e,const source_context * ctx)
    {
        source_context * newCtx = ctx ? ctx->clone() : NULL;
        return new error_info(e,"",newCtx);
    }

    env::object::shared_ptr m_object;
    env::frame * m_frame;
    arguments_container m_args;
    call_serializer m_serializer;
    boost::function<arguments_container::value_type (error_info *)> m_error_handler;
};

} //namespace script
} //namespace fungu

#endif
