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

namespace objectlib{

inline any create_object(env_object::call_arguments & args,env_frame * aFrame)
{
    #if 0
    boost::shared_ptr<json::object> initial_data;
    if(!args.empty())
    {
        any data_arg = args.front();
        args.pop_front();
        if(data_arg.get_type() != typeid(boost::shared_ptr<json::object>))
            throw error(BAD_CAST);
        initial_data = any_cast< boost::shared_ptr<json::object> >(data_arg);
        if(!initial_data) throw error(BAD_CAST);
    }
    return table::create(initial_data.get());
    #endif
    
    return table::create();
}

inline any erase_object_member(env_object::call_arguments & args,env_frame * aFrame)
{
    any object_arg = args.safe_front();
    args.pop_front();
    
    env_object * basecasted = NULL;
    table * object;
    
    if(object_arg.get_type() == typeid(env_object::shared_ptr))
        basecasted = any_cast<env_object::shared_ptr>(object_arg).get();
    else
        basecasted = aFrame->lookup_required_object(object_arg.to_string());
    
    object = dynamic_cast_derived<table *>(basecasted->lookup_member(".this"));
    if(!object) throw error(INVALID_TYPE,boost::make_tuple(&typeid(table *)));
    
    std::string membername = args.safe_casted_front<const_string>().copy();
    args.pop_front();
    
    return (bool)object->erase_member(membername);
}

inline any add_object_member(env_object::call_arguments & args,env_frame * aFrame)
{
    any object_arg = args.safe_front();
    args.pop_front();
    
    env_object * basecasted = NULL;
    table * object;
    
    if(object_arg.get_type() == typeid(env_object::shared_ptr))
        basecasted = any_cast<env_object::shared_ptr>(object_arg).get();
    else
        basecasted = aFrame->lookup_required_object(object_arg.to_string());
    
    object = dynamic_cast_derived<table *>(basecasted->lookup_member(".this"));
    if(!object) throw error(INVALID_TYPE,boost::make_tuple(&typeid(table *)));
    
    std::string membername = args.safe_casted_front<const_string>().copy();
    args.pop_front();
    
    any value;
    if(!args.empty())
    {
        value = args.front();
        args.pop_front();
    }
    
    return object->assign(membername,value);
}

} //namespace detail

void register_object_functions(env & environment)
{
    static function<raw_function_type> create_object_func(objectlib::create_object);
    environment.bind_global_object(&create_object_func,FUNGU_OBJECT_ID("object"));
    
    static function<raw_function_type> erase_member_func(objectlib::erase_object_member);
    environment.bind_global_object(&erase_member_func,FUNGU_OBJECT_ID("erase-object-member"));
    
    static function<raw_function_type> add_member_func(objectlib::add_object_member);
    environment.bind_global_object(&add_member_func,FUNGU_OBJECT_ID("add-object-member"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
