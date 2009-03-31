/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_TABLE_HPP
#define FUNGU_SCRIPT_TABLE_HPP

#include "env.hpp"
#include "variable.hpp"
#include <map>
#include <string>

namespace fungu{
namespace script{

class table:public env::object
{
public:
    table();
    static shared_ptr create();
    result_type apply(apply_arguments & args,frame *);
    shared_ptr assign(const std::string & name,const any & data);
    void assign(const any & source);
    object * lookup_member(const_string id);
    bool erase_member(const std::string & name);
private:
    typedef std::map<std::string,shared_ptr> map;
    map m_members;
};

} //namespace script
} //namespace fungu

#endif
