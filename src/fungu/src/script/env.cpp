/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"

#include "fungu/string.hpp"
#include "fungu/script/error.hpp"
#include "fungu/script/any.hpp"
#include "fungu/script/json.hpp"
#include "fungu/script/lexical_cast.hpp"
#include "fungu/script/result_type.hpp"
#include "fungu/script/arguments_container.hpp"
#include "fungu/script/symbol_table.hpp"

#include <typeinfo>
#include <map>
#include <list>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/function.hpp>

namespace fungu{
namespace script{

env::env():m_global(this), m_source_ctx(NULL), m_recursion_limit(FUNGU_SCRIPT_DEFAULT_RECURSION_LIMIT)
{
    
}
    
env::~env()
{
    
}

env::frame * env::get_global_scope()
{
    return &m_global;
}

const env::frame * env::get_global_scope()const
{
    return &m_global;
}

bool env::bind_infix_operator(env::object_identifier infix_id,object_identifier mapped_id)
{
    if(!m_global.lookup_object(mapped_id)) return false;
    m_infix_operators[infix_id] = mapped_id;
    return true;
}

bool env::is_infix_operator(env::object_identifier id)const
{
    return m_infix_operators.count(id);
}

env::object_identifier env::get_infix_operator_mapping(env::object_identifier id)const
{
    return m_infix_operators.find(id)->second;
}

env::frame::frame_symbol ** env::lookup_symbol(env::object_identifier id)
{
    return get_symbol_table().lookup_symbol(id);
}
    
env::frame::frame_symbol ** env::register_symbol(env::object_identifier id)
{
    return get_symbol_table().register_symbol(id);
}

void env::set_source_context(const source_context * source_ctx)
{
    m_source_ctx = source_ctx;
}

const source_context * env::get_source_context() const
{
    return m_source_ctx;
}

env::symbol & env::bind_global_object(env::object * obj, env::object_identifier id)
{
    return m_global.bind_global_object(obj,id);
}

env::symbol & env::bind_universal_object(env::object * obj, env::object_identifier id)
{
    return (new frame::frame_symbol(env::register_symbol(id), NULL))
        ->attach_to_env().bind_object(obj).set_dynamically_scoped();
}

bool env::is_valid_symbol_handle(symbol_handle handle)
{
    return handle && *handle;
}

env::symbol_handle env::invalid_symbol_handle()
{
    return NULL;
}

int env::get_recursion_limit()const
{
    return m_recursion_limit;
}

env::observer_function env::unset_bind_observer()
{
    observer_function old_function = m_bind_observer;
    m_bind_observer = observer_function();
    return old_function;
}

#ifdef FUNGU_WITH_LUA
lua_State * env::get_lua_state()const
{
    return m_lua_state;
}

void env::set_lua_state(lua_State * state)
{
    m_lua_state = state;
}
#endif

symbol_table<env::frame::frame_symbol *> & env::get_symbol_table()
{
    static symbol_table<frame::frame_symbol *> sm_symbol_table(NULL);
    return sm_symbol_table;
}

} //namespace script
} //namespace fungu
