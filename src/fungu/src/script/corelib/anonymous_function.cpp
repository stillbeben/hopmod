/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/env.hpp"

#include "fungu/script/corelib/anonymous_function.hpp"

#include "fungu/script/function.hpp"
#include "fungu/script/code_block.hpp"
#include "fungu/script/parse_array.hpp"
#include "fungu/script/varg_setter.hpp"
#include "fungu/script/any_variable.hpp"

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

class anonymous_function:public env::object
{
public:
    anonymous_function(const std::vector<const_string> & params, const_string code, frame * aFrame)
     :m_first_param(NULL), 
      m_last_param(NULL),
      m_code(code, aFrame->get_env()->get_source_context()),
      m_recursion_depth(0)
    {
        m_code.compile(aFrame);
        m_closure = get_closure(aFrame);
        
        m_last_param = m_closure->get_last_bind();
        for(int i = params.size() - 1; i >= 0; i--)
        {
            m_closure->bind_object(new any_variable, params[i].copy()).adopt_object();
            m_first_param = m_closure->get_last_bind();
        }
        
        any_variable * callee_var = new any_variable;
        callee_var->assign(get_shared_ptr());
        m_closure->bind_object(callee_var,FUNGU_OBJECT_ID("callee")).adopt_object();
    }
    
    ~anonymous_function()
    {
        env::closure_frame::delete_(m_closure);
    }
    
    result_type apply(apply_arguments & args,frame * aFrame)
    {
        #define ANONYMOUS_FUNCTION_APPLY_CLEANUP \
            m_closure->detach_bindings_from_env(); \
            m_closure->unset_return(); \
            m_recursion_depth--;
        
        //initialisation
        if(++m_recursion_depth == 1) m_closure->attach_bindings_to_env();
        else if(m_recursion_depth > aFrame->get_env()->get_recursion_limit())
            throw error(HIT_RECURSION_LIMIT,boost::make_tuple(aFrame->get_env()->get_recursion_limit()));
        
        //mapping arguments to parameters
        for( env::frame::frame_symbol * param = m_first_param; 
             param && param != m_last_param; param = param->get_next_sibling())
        {
            if(args.empty())
            {
                ANONYMOUS_FUNCTION_APPLY_CLEANUP;
                throw error(NOT_ENOUGH_ARGUMENTS);
            }
            static_cast<any_variable *>(param->get_object())->assign(args.front());
            args.pop_front();
        }
        
        //run instructions
        result_type result;
        try
        {
            code_block::iterator instruction = m_code.begin();
            for(; instruction != m_code.end() && !m_closure->has_expired(); ++instruction)
            {
                result = instruction->eval(m_closure);
            }
        }
        catch(error_exception)
        {
            ANONYMOUS_FUNCTION_APPLY_CLEANUP;
            throw;
        }
        catch(error_exception *)
        {
            ANONYMOUS_FUNCTION_APPLY_CLEANUP;
            throw;
        }
        
        if(!m_closure->get_result_value().empty())
            result = m_closure->get_result_value();
        
        ANONYMOUS_FUNCTION_APPLY_CLEANUP;
        
        return result;
    }
    
    #ifdef FUNGU_WITH_LUA
    int apply(lua_State * L)
    {
        apply_arguments args;
        int nargs = lua_gettop(L);
        for(int i = 1; i <= nargs; i++)
        {
            size_t len;
            const char * argi = lua_tolstring(L, i, &len);
            args.push_back(const_string(argi,argi+len-1));
        }
        
        lua_getfield(L, LUA_REGISTRYINDEX, "fungu_script_global_frame");
        env::frame * aFrame = reinterpret_cast<env::frame *>(lua_touserdata(L, -1));
        if(!aFrame) return luaL_error(L, "missing 'fungu_script_global_frame' field in registry");
        
        result_type result = apply(args,aFrame);
        if(result.empty()) return 0;
        else
        {
            lua_pushstring(L, result.to_string().copy().c_str());
            return 1;
        }
    }
    #endif
    
    result_type value()
    {
        return get_shared_ptr();
    }
    
    const source_context * get_source_context()const
    {
        return m_code.get_source_context();
    }
    
    static result_type define_anonymous_function(apply_arguments & args, frame * aFrame)
    {
        std::vector<const_string> params;
        parse_array<std::vector<const_string>,true>(
            args.safe_casted_front<const_string>(), aFrame, params);
        args.pop_front();
        
        const_string code = args.safe_casted_front<const_string>();
        args.pop_front();
        
        anonymous_function * func_obj = NULL;
        try
        {
            func_obj = new anonymous_function(params, code, aFrame);
        }
        catch(error_info *){delete func_obj; throw;}
        catch(error_exception){delete func_obj; throw;}
        
        func_obj->set_adopted_flag();
        return func_obj->get_shared_ptr();
    }
private:
    env::closure_frame * get_closure(frame * aFrame)
    {
        if(aFrame->get_closure_refcount()) return env::closure_frame::new_inner(aFrame);
        else return env::closure_frame::new_(aFrame->get_env());
    }
    
    env::closure_frame * m_closure;
    env::frame::frame_symbol * m_first_param;
    env::frame::frame_symbol * m_last_param;
    code_block m_code;
    int m_recursion_depth;
};

} //namespace detail

void register_anonfunc_functions(env & environment)
{
    static function<raw_function_type> define_anonfun_func(detail::anonymous_function::define_anonymous_function);
    environment.bind_global_object(&define_anonfun_func,FUNGU_OBJECT_ID("func"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
