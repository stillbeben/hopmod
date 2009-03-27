#ifndef HOPMOD_SCRIPTING_HPP
#define HOPMOD_SCRIPTING_HPP

#include <fungu/script.hpp>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#define CUBESCRIPT_EXTENSION ".csl"
#define STARTUP_SCRIPT "conf/server.conf"
#define CATCH_ERRORS(code) try{ code; }catch{fungu::script::error_info * e){report_script_error(e);}

void init_scripting();
void shutdown_scripting();
fungu::script::env & get_script_env();
std::string get_script_error_message(fungu::script::error_info * errinfo);
void report_script_error(fungu::script::error_info *);
void register_lua_function(lua_CFunction,const char *);

#endif
