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
#include "lexical_cast.hpp"
#include "result_type.hpp"
#include "arguments_container.hpp"

#include <boost/unordered_map.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/function.hpp>
#include <stack>

#ifdef FUNGU_WITH_LUA
extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#endif

#define FUNGU_OBJECT_ID(id) fungu::const_string(FUNGU_LITERAL_STRING(id))

namespace fungu{
namespace script{

/**
    @brief Global environment class.
    
    Stores a symbol table and other global state for a scripting environment.
*/
class env
{
public:
    typedef const_string object_id;
    static const int max_modules = 4;
    static const int recursion_limit = 1000;
    
    class object;
    class frame;
    class symbol;
    class symbol_local;
    
    #define FUNGU_CUBESCRIPT_ENV_NESTED_CLASS
    #include "env_object.hpp"
    #include "env_symbol.hpp"
    #include "env_frame.hpp"
    #undef FUNGU_CUBESCRIPT_ENV_NESTED_CLASS
    
    typedef boost::function2<void, const_string, object *> observer_function;
    
    env();
    
    /**
        Symbol objects referenced in the symbol table are deleted.
    */
    virtual ~env();

    /**
        @brief Search for a symbol in the symbol table.
        
        @param id symbol name
        @return pointer to a symbol object
        
        Returns NULL if a symbol is not found.
    */
    symbol * lookup_symbol(object_id id)const;
    
    /**
        @brief Create a symbol object and register the name with the symbol
        table.
        
        @param id symbol name
        @return pointer to symbol object.
        
        If the symbol entry already exists then creation of a new symbol object
        is bypassed and a pointer to the existing symbol object is returned.
    */
    symbol * create_symbol(object_id id);
    
    /**
        @brief Create a global symbol binding.
        
        @param obj pointer of the object to bind
        @param id symbol name
        
        The operation is always successful; an existing symbol binding with the
        same name is overridden.
    */
    void bind_global_object(object * obj, object_id id);
    
    /**
        @brief Search for a global symbol binding.
        
        @param id symbol name
        @return pointer to the bound object
        
        Returns NULL if the symbol is not found.
    */
    object * lookup_global_object(object_id id)const;
    
    /**
        @brief Set associated source_context object.
    */
    void set_source_context(const source_context * source_ctx);
    
    /**
        @brief Get associated source_context object.
        
        Get information about the source code location of the current
        expression being evaluated.
    */
    const source_context * get_source_context()const;
    
    /**
        @brief Set a callback function to observe the creation of global symbol
        bindings.
        
        @param new_function Observer function; something than can be called as new_function(id, object).
        @return The old observer function as a boost::function2<void, const_string, object *> object.
        
        The observer function is called from the bind_global_object method.
    */
    template<typename ObserverFunction>
    observer_function set_bind_observer(ObserverFunction new_function)
    {
        observer_function old_function = m_bind_observer;
        m_bind_observer = new_function;
        return old_function;
    }
    
    /**
        @brief Unset observer function set by set_bind_observer method.
        @return The observer function that was set, returned as a boost::function2<void, const_string, object *> object.
    */
    observer_function unset_bind_observer();
    
    #ifdef FUNGU_WITH_LUA
    /**
        @brief Get associated lua state.
    */
    lua_State * get_lua_state()const;
    
    /**
        @brief Set associated lua state.
    */
    void set_lua_state(lua_State * state);
    #endif
        
    template<typename ModuleClass>
    ModuleClass * get_module_instance()const
    {
        int id = ModuleClass::get_module_id();
        assert(id >= 0 && id < max_modules);
        void * vptr = m_module[id];
        if(!vptr) return NULL;
        return reinterpret_cast<ModuleClass *>(vptr);
    }
    
    template<typename ModuleClass>
    void set_module_instance(ModuleClass * mod)
    {
        int id = ModuleClass::get_module_id();
        assert(id >= 0 && id < max_modules);
        m_module[id] = mod;
    }
    
    template<typename ModuleClass>
    void unset_module_instance()
    {
        int id = ModuleClass::get_module_id();
        assert(id >= 0 && id < max_modules);
        m_module[id] = NULL;
    }
    
    static int generate_module_id()
    {
        static int module_count = 0;
        assert(module_count < max_modules); //increase max_modules if this ever fails
        return module_count++;
    }
    
    void push_arg(any);
    any & top_arg();
    void pop_arg();
    int get_arg_count()const;
private:
    boost::unordered_map<const_string, symbol *> m_symbol;
    
    std::stack<any> m_arg;
    
    const source_context * m_source_ctx;
    int m_recursion_limit;
    
    observer_function m_bind_observer;
    
    #ifdef FUNGU_WITH_LUA
    lua_State * m_lua_state;
    #endif
    
    void * m_module[max_modules];
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
