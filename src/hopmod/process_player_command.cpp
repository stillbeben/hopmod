#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "cube.h"
#include "tools.h"
#include "hopmod.hpp"

#include <fungu/script/env.hpp>
#include <fungu/script/callargs.hpp>
#include <fungu/script/error.hpp>
using namespace fungu;

namespace server{
void player_msg(int,const char *);
} //namespace server

bool using_command_prefix = true;
char command_prefix = '#';

bool eval_player_command(int cn,const char * cmdline, const std::vector<std::string> & arguments, script::env & env)
{
    if(arguments.empty()) return false;
    
    defformatstring(cmdname)("playercmd_%s",arguments[0].c_str());
    script::env_object * cmdobj = env.lookup_global_object(const_string(cmdname,cmdname+strlen(cmdname)-1));
    if(cmdobj)
    {
        std::vector<script::any> args;
        script::callargs callargs(args);
        
        callargs.push_back(cn);
        
        for(std::vector<std::string>::const_iterator it = arguments.begin() + 1; it != arguments.end(); ++it)
            callargs.push_back(const_string(it->c_str(),it->c_str()+it->length() - 1));
        
        try{
            script::env_frame frame(&env);
            cmdobj->call(callargs, &frame);
        }
        catch(script::error_trace * errinfo)
        {
            report_script_error(errinfo);
            server::player_msg(cn, RED "Command Error!");
        }
        catch(script::error err)
        {
            report_script_error(new script::error_trace(err,"",
                (env.get_source_context() ? env.get_source_context()->clone() : NULL)));
            server::player_msg(cn, RED "Command Error!");
        }
    }
    else if(using_command_prefix) server::player_msg(cn, RED "Command not found.");
    
    return true;
}

bool process_player_command(int cn,const char * cmdline)
{
    if(using_command_prefix && cmdline[0] != command_prefix) return false;
    std::vector<std::string> args = parse_player_command_line(cmdline + (using_command_prefix ? 1 : 0));
    eval_player_command(cn,cmdline,args, get_script_env());
    return true;
}
