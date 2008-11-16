/*   libcubescript - A CubeScript engine.  */
/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef LIBCUBESCRIPT_ERROR_HPP
#define LIBCUBESCRIPT_ERROR_HPP

#include <exception>
#include <string>
#include <map>

namespace cubescript{

class error_key
{
public:
    error_key(const char * key):m_key(key){}
    const char * get_key()const{return m_key;}
private:
    const char * m_key;
};

class error_context
{
public:
    error_context(const error_key & key):m_key(key),m_prev(NULL){}
    error_context(error_context * prev_frame):m_key(prev_frame->m_key),m_prev(prev_frame){}
    virtual ~error_context(){if(m_prev) delete m_prev;}
    
    const char * get_key()const{return m_key.get_key();}
    const error_context * get_subcontext()const{return m_prev;}
    
    const error_context * get_root()const
    {
        const error_context * node=this;
        while(node->m_prev) node=node->m_prev;
        return node;
    }
    
    virtual std::string get_name()const=0;
    virtual std::string get_attribute(const std::string &)const=0;
    virtual std::string get_description()const=0;
private:
    error_key m_key;
    error_context * m_prev;
};

std::string format_error_report(const error_context *);

} //namespace cubescript

#endif
