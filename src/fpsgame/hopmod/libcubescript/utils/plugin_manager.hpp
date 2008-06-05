
#ifndef PLUGIN_MANAGER_HPP
#define PLUGIN_MANAGER_HPP

#include "plugin.hpp"
#include <map>

namespace cubescript
{

struct loaded_plugin
{
    plugin * m_plugin;
    void * m_handle;
};

class plugin_manager
{
public:
    void register_functions(domain *);
private:
    void_ load_wrapper(std::list<std::string> &,domain *);
    void load(const std::string &,domain *);
    std::map<std::string,loaded_plugin> m_plugins;
};

} //namespace cubescript

#endif
