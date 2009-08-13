#ifndef HOPMOD_SCRIPTING_HPP
#define HOPMOD_SCRIPTING_HPP

extern "C"{
    #include <lua.h>
}

#include <fungu/script.hpp>

#define CUBESCRIPT_EXTENSION ".csl"
#define STARTUP_SCRIPT "conf/server.conf"
#define CATCH_ERRORS(code) try{ code; }catch{fungu::script::error_trace * e){report_script_error(e);}

void init_scripting();
void shutdown_scripting();
fungu::script::env & get_script_env();
std::string get_script_error_message(fungu::script::error_trace * errinfo);
void report_script_error(fungu::script::error_trace *);
void register_lua_function(lua_CFunction,const char *);
void unset_global(const char *);

#endif
