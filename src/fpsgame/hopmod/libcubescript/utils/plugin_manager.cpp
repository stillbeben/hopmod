
#include "plugin_manager.hpp"
#include <boost/bind.hpp>
#include <dlfcn.h>

namespace cubescript
{
    
void plugin_manager::register_functions(domain * aDomain)
{
    static functionX<void_> func_load(boost::bind(&plugin_manager::load_wrapper,this,_1,_2));
    aDomain->register_symbol("plugin",&func_load);
}

void_ plugin_manager::load_wrapper(std::list<std::string> & args,domain * aDomain)
{
    std::string filename=functionN::pop_arg<std::string>(args);
    load(filename,aDomain);
}

void plugin_manager::load(const std::string & filename,domain * aDomain)
{
    loaded_plugin lp;
    
    std::map<std::string,loaded_plugin>::iterator it=m_plugins.find(filename);
    
    if(it==m_plugins.end())
    {
        loaded_plugin entry;
        entry.m_handle=dlopen(filename.c_str(),RTLD_LAZY);
        
        if(!entry.m_handle) throw runtime_error("runtime.plugin_loader.dlopen_failed");
        
        typedef plugin * (* startup_function)();
        
        startup_function get_csplugin_object=(startup_function)dlsym(entry.m_handle,"get_csplugin_object");
        if(!get_csplugin_object) throw runtime_error("runtime.plugin_loader.function_missing");
        
        entry.m_plugin=get_csplugin_object();
        
        m_plugins[filename]=entry;
        lp=entry;
    }
    else lp=it->second;
    
    plugin_domain_accessor dacc(aDomain);
    lp.m_plugin->register_symbols(&dacc);
}

}//namespace cubescript
