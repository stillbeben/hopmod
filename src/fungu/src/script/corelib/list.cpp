/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/script/env.hpp"
#include "fungu/script/function.hpp"
#include "fungu/script/any_variable.hpp"
#include <boost/shared_ptr.hpp>

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

struct listnode;
typedef boost::shared_ptr<listnode> listnode_ptr;
    
struct listnode
{
    any_variable data;
    listnode_ptr next;
    
    static listnode_ptr constructor(any d,listnode_ptr n)
    {
        listnode_ptr ptr(new listnode);
        ptr->data.assign(d);
        ptr->next = n;
        return ptr;
    }
    
    static any first(listnode_ptr list)
    {
        if(!list) return any::null_value();
        return list->data.value();
    }
    
    static listnode_ptr rest(listnode_ptr list)
    {
        if(!list) return emptylist();
        return list->next;
    }
    
    static listnode_ptr emptylist()
    {
        return listnode_ptr();
    }
    
    static bool listp(listnode_ptr list)
    {
        return list;
    }
};

} //namespace detail

void register_list_functions(env & environment)
{
    static function<detail::listnode_ptr (any,detail::listnode_ptr)> cons_func(detail::listnode::constructor);
    environment.bind_global_object(&cons_func,FUNGU_OBJECT_ID("cons"));
    
    static function<any (detail::listnode_ptr)> first_func(detail::listnode::first);
    environment.bind_global_object(&first_func,FUNGU_OBJECT_ID("first"));
    
    static function<detail::listnode_ptr (detail::listnode_ptr)> rest_func(detail::listnode::rest);
    environment.bind_global_object(&rest_func,FUNGU_OBJECT_ID("rest"));
    
    static function<detail::listnode_ptr ()> emptylist_func(detail::listnode::emptylist);
    environment.bind_global_object(&emptylist_func,FUNGU_OBJECT_ID("empty-list"));
    
    static function<bool (detail::listnode_ptr)> listp_func(detail::listnode::listp);
    environment.bind_global_object(&listp_func,FUNGU_OBJECT_ID("non-empty-list?"));
}

} //namespace corelib
} //namespace script
} //namespace fungu

