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
#ifndef LIBCUBESCRIPT_POINTER_HPP
#define LIBCUBESCRIPT_POINTER_HPP

#include <boost/shared_ptr.hpp>
#include "predefs.hpp"
#include "error.hpp"
#include "lexcast.hpp"
#include "domain.hpp"

namespace cubescript{

template<typename T>
class pointer:public symbol
{
public:
    pointer(const boost::shared_ptr<T> & ptr):m_ptr(ptr){}
    std::string apply(std::list<std::string> & arglist,domain * aDomain)
    {
        std::string src_name=parse_type<std::string>(arglist.front()); arglist.pop_front();
        pointer<T> * ptrwrapper=aDomain->get_symbol_of_kind< pointer<T> >(src_name);
        m_ptr=ptrwrapper->m_ptr;
        return "";
    }
    std::string value()const
    {
        throw error_key("runtime.pointer.no_value");
        //TODO use if print_type specialization exists: return print_type<element_type>(*m_ptr);
    }
    boost::shared_ptr<T> & get(){return m_ptr;}
private:
    boost::shared_ptr<T> m_ptr;
};

} //namespace cubescript

#endif
