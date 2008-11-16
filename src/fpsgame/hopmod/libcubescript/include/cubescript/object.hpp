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
#ifndef LIBCUBESCRIPT_OBJECT_HPP
#define LIBCUBESCRIPT_OBJECT_HPP

#include "symbol.hpp"
#include "function.hpp"

namespace cubescript{

class proto_object:public symbol
{
public:
    proto_object();
    std::string apply(std::list<std::string> &,domain *);
    std::string value()const;
    virtual proto_object * clone()const=0;
protected:
    explicit proto_object(proto_object *); //proto_object inheritance
    proto_object(const proto_object &); //normal copy constructor
    proto_object(domain *);
    void add_member(const char *,symbol *);
    domain * get_parent_domain()const;
private:
    void register_members();
    
    function1<std::string,const std::string &> m_func_value;
    functionV<std::string> m_func_super;
    function2<void,const std::string &,const std::string &> m_func_function;

    std::string get_member_value(const std::string &)const;
    std::string call_super_operation(std::list<std::string> &,domain *);
    void create_function(const std::string &,const std::string &);
    
    std::string m_operation;
    domain m_members;
};

class object_adaptor:public proto_object
{
public:
    object_adaptor(domain *);
    object_adaptor(const object_adaptor &);
    proto_object * clone()const;
};

class null:public symbol
{
public:
    std::string apply(std::list<std::string> &,domain *);
    std::string value()const;
};

} //namespace cubescript

#endif
