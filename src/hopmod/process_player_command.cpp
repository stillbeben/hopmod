#include "cube.h"
#include "tools.h"
#include "hopmod.hpp"
#include "player_command.hpp"

bool eval_player_command(int cn,const char * cmdline, const std::vector<std::string> & arguments,fungu::script::env::frame * parent_frame)
{
    if(arguments.empty()) return false;
    
    fungu::script::env::frame frame(parent_frame);
    
    defformatstring(cmdfile)("./script/command/%s" CUBESCRIPT_EXTENSION ,arguments[0].c_str());
    if(fileexists(cmdfile,"r"))
    {
        try{
            fungu::script::execute_file(cmdfile,&frame);
        }
        catch(fungu::script::error_info * errinfo)
        {
            report_script_error(errinfo);
            server::player_msg(cn, RED "Command Error!");
        }
    }
    else
    {
        defformatstring(cmdname)("playercmd_%s",arguments[0].c_str());
        fungu::script::env::object * cmdobj = parent_frame->lookup_object(fungu::const_string(cmdname,cmdname+strlen(cmdname)-1));
        if(cmdobj)
        {
            fungu::script::arguments_container apply_args;
            apply_args.push_back(cn);
            for(std::vector<std::string>::const_iterator it = arguments.begin() + 1; it != arguments.end(); ++it)
                apply_args.push_back(fungu::const_string(it->c_str(),it->c_str()+it->length() - 1));
            
            try{
                cmdobj->apply(apply_args,&frame);
            }
            catch(fungu::script::error_info * errinfo)
            {
                report_script_error(errinfo);
                server::player_msg(cn, RED "Command Error!");
            }
            catch(fungu::script::error err)
            {
                report_script_error(new fungu::script::error_info(err,"",
                    (frame.get_env()->get_source_context() ? frame.get_env()->get_source_context()->clone() : NULL)));
                server::player_msg(cn, RED "Command Error!");
            }
        }
        else server::player_msg(cn, RED "Command not found.");
    }
    
    return true;
}

bool process_player_command(int cn,const char * cmdline)
{
    if(cmdline[0]!='#') return false;
    std::vector<std::string> args = parse_player_command_line(cmdline+1);
    eval_player_command(cn,cmdline,args, get_script_env().get_global_scope());
    return true;
}
