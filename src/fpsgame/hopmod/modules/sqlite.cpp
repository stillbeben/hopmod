
#include "../module.hpp"
#include <sqlite3.h>

class sqlite_module:public cubescript::module
{
public:
    void register_symbols(cubescript::module_domain_accessor * domain)
    {
        static cubescript::functionV<void_> func_open(boost::bind(&sqlite_module::open,this));
        aDomain->register_symbol("sqlite_open",&func_open);
        aDomain->register_symbol("sqlite_exec",&func_exec);
    }
private:
    typedef boost::shared_ptr<sqlite3> handle;
    
    void_ open(std::list<std::string> & arglist,cubescript::domain * aDomain)
    {
        std::string filename=functionN::pop_arg<std::string>(arglist);
        std::string id=functionN::pop_arg<std::string>(arglist);
        sqlite3 * handle=NULL;
        if(sqlite3_open(filename.c_str(),&handle)!=SQLITE_OK) throw cubescript::error_key("runtime.function.sqlite_open.failed");
        cubescript::bind< cubescript::pointer<sqlite3> >(handle,id.c_str(),aDomain);
    }
};

static sqlite_module modinst;
cubescript::module * get_csmodule_object()
{
    return &modinst;
}
