/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "fungu/script/pch.hpp"
#endif

#include "fungu/script/env.hpp"

#include "fungu/script/corelib/anonymous_function.hpp"

#include "fungu/script/function.hpp"
#include "fungu/script/code_block.hpp"
#include "fungu/script/parse_array.hpp"
#include "fungu/script/any_variable.hpp"

namespace fungu{
namespace script{
namespace corelib{

namespace detail{

class anonymous_function:public env::object
{
public:
    anonymous_function(const std::vector<const_string> & params, const_string code, frame * aFrame)
     :m_code(code, aFrame->get_env()->get_source_context()),
      m_recursion_depth(0)
    {
        m_code.compile(aFrame);
        
        m_params.reserve(params.size());
        for(std::vector<const_string>::const_iterator it = params.begin(); it != params.end(); ++it)
            m_params.push_back(aFrame->get_env()->create_symbol(*it));
        
        m_callee_symbol = aFrame->get_env()->create_symbol(FUNGU_OBJECT_ID("callee"));
    }
    
    ~anonymous_function()
    {
        
    }
    
    result_type call(call_arguments & callargs, frame * aFrame)
    {
        if(++m_recursion_depth > env::recursion_limit)
            throw error(HIT_RECURSION_LIMIT,boost::make_tuple(env::recursion_limit));
        
        if(callargs.size() < m_params.size()) throw error(NOT_ENOUGH_ARGUMENTS);
        
        frame func_frame(aFrame->get_env());
        
        func_frame.bind_object(get_shared_ptr().get(), m_callee_symbol);
        
        std::vector<any_variable> arg(m_params.size());
        
        std::vector<any_variable>::iterator argIt = arg.begin();
        std::vector<env::symbol *>::iterator paramIt = m_params.begin();
        
        for(; paramIt != m_params.end(); ++argIt, ++paramIt)
        {
            (*argIt).set_temporary();
            (*argIt).assign(callargs.front());
            
            func_frame.bind_object(&(*argIt), *paramIt);
            
            callargs.pop_front();
        }
        
        try
        {
            result_type result = m_code.eval_each_expression(&func_frame);
            m_recursion_depth--;
            return result;
        }
        catch(...)
        {
            m_recursion_depth--;
            throw;
        }
    }
    
    //TODO REMOVE
    #ifdef FUNGU_WITH_LUA
    int call(lua_State * L)
    {
        std::vector<any> args;
        int nargs = lua_gettop(L);
        for(int i = 1; i <= nargs; i++)
        {
            size_t len;
            const char * argi = lua_tolstring(L, i, &len);
            args.push_back(const_string(argi,argi+len-1));
        }
        
        lua_getfield(L, LUA_REGISTRYINDEX, "fungu_script_env");
        env * environment = reinterpret_cast<env *>(lua_touserdata(L, -1));
        if(!environment) return luaL_error(L, "missing 'fungu_script_env' field in lua registry");
        
        env::frame callframe(environment);
        call_arguments callargs(args);
        result_type result = call(callargs, &callframe);
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
    
    static result_type define_anonymous_function(call_arguments & args, frame * aFrame)
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
        catch(error_trace *){delete func_obj; throw;}
        catch(error_exception){delete func_obj; throw;}
        
        func_obj->set_adopted();
        return func_obj->get_shared_ptr();
    }
private:
    code_block m_code;
    std::vector<env::symbol *> m_params;
    int m_recursion_depth;
    env::symbol * m_callee_symbol;
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
