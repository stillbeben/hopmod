/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/table.hpp"
#include "fungu/script/any_variable.hpp"

namespace fungu{
namespace script{

table::table(const json::object * source)
{
    if(source)
    {
        for(json::object::const_iterator it = source->begin();
            it != source->end(); it++)
        {
            any_variable * a = new any_variable;
            a->set_adopted_flag();
            if(it->second->get_type() == typeid(json::object))
                a->assign(create(any_cast<json::object>(it->second.get())));
            else a->assign(*it->second);
            m_members[it->first] = a->get_shared_ptr();
        }
    }
    
    m_members[".this"] = this->get_shared_ptr();
}

env::object::shared_ptr table::create(const json::object * source)
{
    table * t = new table(source);
    t->set_adopted_flag();
    return t->get_shared_ptr();
}

result_type table::apply(apply_arguments & args,frame *)
{
    object::shared_ptr obj = this;
    while(!args.empty())
    {
        const_string member_id = args.casted_front<const_string>();
        obj = obj->lookup_member(member_id);
        args.pop_front();
        if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(member_id.copy()));
    }
    return obj->get_shared_ptr();
}

result_type table::value()
{
    std::stringstream output;
    int shown = 0;
    output<<"{";
    for(map::const_iterator it = m_members.begin();
        it != m_members.end(); ++it)
    {
        if(it->first[0]=='.') continue; //hidden member
        output<<(!shown++ ? "" : ",")<<"\""<<it->first<<"\":";
        const any & var = static_cast<any_variable *>(it->second.get())->get_any();
        bool is_table = var.get_type() == typeid(shared_ptr) && 
            typeid(*any_cast<shared_ptr>(var).get()) == typeid(table);
        if(is_table) output<<any_cast<std::string>(any_cast<shared_ptr>(var)->value());
        else output<<lexical_cast_detail::write_json_value(&var);
    }
    output<<"}";
    return output.str();
}

env::object::shared_ptr table::assign(const std::string & name,const any & data)
{
    any_variable * anyvar = new any_variable;
    anyvar->set_adopted_flag();
    anyvar->assign(data);
    m_members[name] = anyvar->get_shared_ptr();
    return anyvar->get_shared_ptr();
}

void table::assign(const any & source)
{
    if(source.get_type() != typeid(shared_ptr)) throw error(NO_CAST);
    const table * source_table = dynamic_cast<const table *>(any_cast<shared_ptr>(source).get());
    if(!source_table) throw error(NO_CAST);
    m_members = source_table->m_members;
}

env::object * table::lookup_member(const_string id)
{
    map::iterator it = m_members.find(id.copy());
    if(it == m_members.end()) return NULL;
    return it->second.get();
}

bool table::erase_member(const std::string & name)
{
    return m_members.erase(name);
}

} //namespace script
} //namespace fungu
