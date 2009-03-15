/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ENV_HPP
#define FUNGU_SCRIPT_ENV_HPP

#include "../string.hpp"
#include "error.hpp"
#include "any.hpp"
#include "json.hpp"
#include "lexical_cast.hpp"
#include "result_type.hpp"
#include "arguments_container.hpp"
#include "symbol_table.hpp"
#include "anyvar_macro.hpp"

#include <map>
#include <boost/intrusive_ptr.hpp>
#include <boost/function.hpp>

#ifdef FUNGU_WITH_LUA
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#endif

#define FUNGU_OBJECT_ID(id) fungu::const_string(FUNGU_LITERAL_STRING(id))
#define FUNGU_RUNTIME_OBJECT_ID(id) FUNGU_OBJECT_ID(id)

#define FUNGU_SCRIPT_DEFAULT_RECURSION_LIMIT 1000

namespace fungu{
namespace script{

/**
    
*/
class env
{
public:
    typedef const_string object_identifier;
    
    class object;
    class frame;
    class symbol;
    
    #define FUNGU_CUBESCRIPT_ENV_NESTED_CLASS
    #include "env_object.hpp"
    #include "env_symbol.hpp"
    #include "env_frame.hpp"
    #undef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS
    
    typedef frame::frame_symbol * const * symbol_handle;
    typedef boost::function2<void, const_string, object *> observer_function;
    
    env();
    virtual ~env();

    frame * get_global_scope();
    const frame * get_global_scope()const;
    
    bool bind_infix_operator(object_identifier infix_id,object_identifier mapped_id);
    bool is_infix_operator(object_identifier id)const;
    object_identifier get_infix_operator_mapping(object_identifier id)const;
    
    static frame::frame_symbol ** lookup_symbol(object_identifier id);
    static frame::frame_symbol ** register_symbol(object_identifier id);
    
    void set_source_context(const source_context * source_ctx);
    const source_context * get_source_context()const;
    
    symbol & bind_global_object(object * obj, object_identifier id);
    static symbol & bind_universal_object(object * obj, object_identifier id);
    
    static bool is_valid_symbol_handle(symbol_handle handle);
    
    static symbol_handle invalid_symbol_handle();
    
    int get_recursion_limit()const;
    
    template<typename ObserverFunction>
    observer_function set_bind_observer(ObserverFunction new_function)
    {
        observer_function old_function = m_bind_observer;
        m_bind_observer = new_function;
        return old_function;
    }
    
    observer_function unset_bind_observer();
    
    #ifdef FUNGU_WITH_LUA
    lua_State * get_lua_state()const;
    void set_lua_state(lua_State * state);
    #endif
private:
    static symbol_table<frame::frame_symbol *> & get_symbol_table();
    
    typedef std::map<object_identifier,object_identifier
        ,object_identifier::less_than_comparator> infix_map;
    infix_map m_infix_operators;
    
    frame m_global;
    const source_context * m_source_ctx;
    int m_recursion_limit;
    
    observer_function m_bind_observer;
    
    #ifdef FUNGU_WITH_LUA
    lua_State * m_lua_state;
    #endif
};

inline void intrusive_ptr_add_ref(env::object * obj)
{
    obj->add_ref();
}

inline void intrusive_ptr_release(env::object * obj)
{
    if(obj->unref().get_refcount()==0 && obj->is_adopted()) delete obj;
}

} //namespace script
} //namespace fungu

#endif
