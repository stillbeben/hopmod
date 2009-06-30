/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{

namespace execlib{

static inline table * get_file_ext_handlers_table()
{
    static table file_ext_handlers;
    return &file_ext_handlers;
}

inline result_type exec_cubescript(env::object::call_arguments & args,env::frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    args.pop_front();
    throw_if_error(execute_file(std::string(filename.copy()).c_str(),*aFrame->get_env()));
    return any::null_value();
}

#ifdef FUNGU_WITH_LUA
inline result_type exec_lua(env::object::call_arguments & args,env::frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    args.pop_front();
    if(luaL_dofile(aFrame->get_env()->get_lua_state(), filename.copy().c_str()))
        throw error(LUA_ERROR,boost::make_tuple(aFrame->get_env()->get_lua_state()));
    return any::null_value();
}
#endif

inline const_string get_filename_extension(const_string filename)
{
    for(const_string::const_iterator it = filename.end() - 1; it != filename.begin(); it--)
        if(*it=='.') return filename.substring(it+1,filename.end()-1);
    return const_string();
}

inline result_type exec_script(env::object::call_arguments & args,env::frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    env::object * handler = get_file_ext_handlers_table()->lookup_member(get_filename_extension(filename));
    if(handler) return handler->call(args, aFrame);
    else return exec_cubescript(args, aFrame);
}

} //namespace detail

void register_exec_functions(env & environment)
{
    static function<raw_function_type> exec_func(execlib::exec_script);
    environment.bind_global_object(&exec_func,FUNGU_OBJECT_ID("exec"));
    
    static function<raw_function_type> exec_cubescript_func(execlib::exec_cubescript);
    environment.bind_global_object(&exec_cubescript_func,FUNGU_OBJECT_ID("exec-cubescript"));
    
    #ifdef FUNGU_WITH_LUA
    static function<raw_function_type> exec_lua_func(execlib::exec_lua);
    environment.bind_global_object(&exec_lua_func,FUNGU_OBJECT_ID("exec-lua"));
    execlib::get_file_ext_handlers_table()->assign("lua",exec_lua_func.get_shared_ptr());
    #endif
    
    environment.bind_global_object(execlib::get_file_ext_handlers_table(), FUNGU_OBJECT_ID("exec-handlers"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
