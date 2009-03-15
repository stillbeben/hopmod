/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/function.hpp"

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

class reference:public env::object
{
public:
    reference(object * object)
    {
        m_object = object->get_shared_ptr();
    }
    
    reference(shared_ptr obj)
     :m_object(obj)
    {
        
    }
    
    ~reference()
    {
        
    }
    
    result_type apply(apply_arguments & args,frame * aScope)
    {
        return m_object->apply(args,aScope);
    }
    
    result_type value()
    {
        return m_object->value();
    }
    
    void assign(const any & src)
    {
        m_object->assign(src);
    }
    
    object * lookup_member(const_string id)
    {
        return m_object->lookup_member(id);
    }
private:
    shared_ptr m_object;
};

inline result_type define_ref(env::object::apply_arguments & args,env::frame * aFrame)
{
    const_string symbol_id = args.safe_casted_front<const_string>();
    args.pop_front();
    
    env::object::shared_ptr obj = aFrame->lookup_required_object(symbol_id)->get_shared_ptr();
    
    while(!args.empty())
    {
        symbol_id = args.casted_front<const_string>();
        obj = obj->lookup_member(symbol_id);
        args.pop_front();
        if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(symbol_id.copy()));
    }
    
    reference * ref = new reference(obj);
    ref->set_adopted_flag();
    return obj;
}

} //namespace detail

void register_reference_functions(env & environment)
{
    static function<raw_function_type> define_ref_func(detail::define_ref);
    environment.bind_global_object(&define_ref_func,FUNGU_OBJECT_ID("ref"));
    
    //TODO weak references
    //static function<raw_function_type> define_weakref_func(detail::define_weakref);
    //environment.bind_global_object(&define_weakref_func,FUNGU_OBJECT_ID("weakref"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
