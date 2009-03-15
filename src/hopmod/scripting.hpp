#ifndef HOPMOD_SCRIPTING_HPP
#define HOPMOD_SCRIPTING_HPP

#include <fungu/script.hpp>

#define STARTUP_SCRIPT "conf/server.conf"
#define CATCH_ERRORS(code) try{ code; }catch{fungu::script::error_info * e){report_script_error(e);}

void init_scripting();
void shutdown_scripting();
fungu::script::env & get_script_env();
void report_script_error(fungu::script::error_info *);

#endif
