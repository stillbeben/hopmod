/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{

namespace listlib{

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
    static function<listlib::listnode_ptr (any,listlib::listnode_ptr)> cons_func(listlib::listnode::constructor);
    environment.bind_global_object(&cons_func,FUNGU_OBJECT_ID("cons"));
    
    static function<any (listlib::listnode_ptr)> first_func(listlib::listnode::first);
    environment.bind_global_object(&first_func,FUNGU_OBJECT_ID("first"));
    
    static function<listlib::listnode_ptr (listlib::listnode_ptr)> rest_func(listlib::listnode::rest);
    environment.bind_global_object(&rest_func,FUNGU_OBJECT_ID("rest"));
    
    static function<listlib::listnode_ptr ()> emptylist_func(listlib::listnode::emptylist);
    environment.bind_global_object(&emptylist_func,FUNGU_OBJECT_ID("empty-list"));
    
    static function<bool (listlib::listnode_ptr)> listp_func(listlib::listnode::listp);
    environment.bind_global_object(&listp_func,FUNGU_OBJECT_ID("non-empty-list?"));
}

} //namespace corelib
} //namespace script
} //namespace fungu

