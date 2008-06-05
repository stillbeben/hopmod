
#ifndef MODULE_LOADER_HPP
#define MODULE_LOADER_HPP

#include "module.hpp"
#include <map>

namespace cubescript
{

class module_loader
{
public:
    module_loader();
    void register_functions(domain *);
private:
    struct loaded_module
    {
        module * m_module;
        void * m_handle;
    };
    void_ load_wrapper(std::list<std::string> &,domain *);
    void load(const std::string &,domain *);
    std::map<std::string,loaded_module> m_modules;
};

} //namespace cubescript

#endif
