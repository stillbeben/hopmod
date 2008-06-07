
#include "module_loader.hpp"
#include <boost/bind.hpp>
#include <dlfcn.h>

namespace cubescript
{

module_loader::module_loader()
{
    std::string paths;
    char * existing=getenv("LD_LIBRARY_PATH");
    if(existing) paths=std::string(existing) + ":";
    paths+="./lib/";
    setenv("LD_LIBRARY_PATH",paths.c_str(),true);
}

void module_loader::register_functions(domain * aDomain)
{
    static functionV<void_> func_load(boost::bind(&module_loader::load_wrapper,this,_1,_2));
    aDomain->register_symbol("load",&func_load);
}

void_ module_loader::load_wrapper(std::list<std::string> & args,domain * aDomain)
{
    std::string filename=functionN::pop_arg<std::string>(args);
    load(filename,aDomain);
    return void_();
}

void module_loader::load(const std::string & filename,domain * aDomain)
{
    loaded_module lp;
    std::map<std::string,loaded_module>::iterator it=m_modules.find(filename);
    
    if(it==m_modules.end())
    {
        std::string real_filename;
        real_filename=std::string("./lib/") + filename + ".csm";
        
        loaded_module entry;
        entry.m_handle=dlopen(real_filename.c_str(),RTLD_LAZY);
        
        if(!entry.m_handle) throw error_key("runtime.module_loader.dlopen_failed");
        
        typedef module * (* startup_function)();
        
        startup_function get_csmodule_object=(startup_function)dlsym(entry.m_handle,"get_csmodule_object");
        if(!get_csmodule_object) throw error_key("runtime.module_loader.function_missing");
        
        entry.m_module=get_csmodule_object();
        
        m_modules[filename]=entry;
        lp=entry;
    }
    else lp=it->second;
    
    module_domain_accessor dacc(aDomain);
    lp.m_module->register_symbols(&dacc);
}

}//namespace cubescript
