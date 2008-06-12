
#include "module_loader.hpp"
#include <boost/bind.hpp>
#include <dlfcn.h>
#include <sys/stat.h>
#include <errno.h>

namespace cubescript
{

bool filesystem_path(const std::string & filename)
{
    struct stat info;
    if(::stat(filename.c_str(),&info)==-1)
    {
        if(errno!=ENOENT) throw error_key("runtime.function.filesystem_path.unexpected_error");
        return false;
    }
    
    return true;
}

class module_loader:public proto_object
{
public:
    module_loader()
     :m_func_module(boost::bind(&module_loader::load_module,this,_1,_2)),
      m_func_ls(boost::bind(&module_loader::get_module_list,this)),
      m_var_paths(m_paths),
      m_var_suffix(m_file_suffix),
      m_file_suffix(".csm")
    {
        add_member("module",&m_func_module);
        add_member("ls",&m_func_ls);
        add_member("paths",&m_var_paths);
        add_member("suffix",&m_var_suffix);
        
        m_paths.push_back(".");
    }
    
    module_loader(const module_loader & src)
     :m_func_module(boost::bind(&module_loader::load_module,this,_1,_2)),
      m_func_ls(boost::bind(&module_loader::get_module_list,this)),
      m_var_paths(m_paths),
      m_var_suffix(m_file_suffix),
      m_modules(src.m_modules),
      m_paths(src.m_paths),
      m_file_suffix(src.m_file_suffix)
    {
        add_member("module",&m_func_module);
        add_member("ls",&m_func_ls);
        add_member("paths",&m_var_paths);
        add_member("suffix",&m_var_suffix);
    }
    
    proto_object * clone()const
    {
        return new module_loader(*this);
    }
    
    _void load_module(std::list<std::string> & arglist,domain * aDomain);
    std::vector<std::string> get_module_list()const;
private:
    functionV<_void> m_func_module;
    function0<std::vector<std::string> > m_func_ls;
    variable_ref< std::list<std::string> > m_var_paths;
    variable_ref< std::string > m_var_suffix;
    
    struct loaded_module
    {
        std::string filename;
        module * _module;
        void * handle;
    };
    typedef std::map<std::string,loaded_module> module_map;
    module_map m_modules;
    
    std::list<std::string> m_paths;
    std::string m_file_suffix;
};

static void module_throw_exception(const error_key & e){throw e;}

_void module_loader::load_module(std::list<std::string> & arglist,domain * aDomain)
{
    std::string vname=functionN::pop_arg<std::string>(arglist);
    
    loaded_module lp;
    std::map<std::string,loaded_module>::iterator it=m_modules.find(vname);
    
    if(it==m_modules.end())
    {
        bool found=false;
        std::list<std::string>::iterator pathIt=m_paths.begin();
        std::string filename;
        while(pathIt!=m_paths.end() && !found)
        {
            filename=(*pathIt)+"/"+vname+m_file_suffix; 
            found=filesystem_path(filename);
            ++pathIt;
        }
        if(!found) throw error_key("runtime.object.load.function.module.file_not_found");
        
        loaded_module entry;
        entry.filename=filename;
        entry.handle=dlopen(filename.c_str(),RTLD_LAZY);
        if(!entry.handle) throw error_key("runtime.object.load.function.module.dlopen_failed");
        
        typedef module * (* startup_function)();
        
        startup_function get_csmodule_object=(startup_function)dlsym(entry.handle,"get_csmodule_object");
        if(!get_csmodule_object) throw error_key("runtime.object.load.function.module.invalid");
        
        entry._module=get_csmodule_object();
        
        m_modules[vname]=entry;
        lp=entry;
        
        lp._module->initialise(&module_throw_exception);
    }
    else lp=it->second;
    
    module::domain_accessor dacc(get_parent_domain());
    lp._module->register_symbols(&dacc);
    
    return _void();
}

std::vector<std::string> module_loader::get_module_list()const
{
    std::vector<std::string> result(m_modules.size());
    int i=0;
    for(module_map::const_iterator it=m_modules.begin(); it!=m_modules.end(); ++it,++i) result[i]=it->first;
    return result;
}

module::domain_accessor::domain_accessor(domain * aDomain):m_domain(aDomain)
{
    
}

void module::domain_accessor::register_symbol(const char * id,symbol * s)
{
    return m_domain->register_symbol(id,s);
}

void register_module_loader(domain * aDomain)
{
    static module_loader object_module_loader;
    aDomain->register_symbol("load",&object_module_loader);
}

} //namespace cubescript
