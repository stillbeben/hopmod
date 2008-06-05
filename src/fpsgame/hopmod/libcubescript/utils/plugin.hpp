
#ifndef PLUGIN_HPP
#define PLUGIN_HPP

#include <cubescript.hpp>

namespace cubescript
{

/*!
    @brief Access proxy to a domain object.
    
    The purpose of this class is to give the plugin class a limited interface
    to the given domain object.
*/
class plugin_domain_accessor
{
public:
    plugin_domain_accessor(domain *);
    void register_symbol(const std::string &,symbol *);
private:
    domain * m_domain;
};

class plugin
{
public:
    virtual void register_symbols(plugin_domain_accessor *)=0;
};

} //namespace cubescript

#ifdef CUBESCRIPT_PLUGIN
extern "C"
{
    cubescript::plugin * get_csplugin_object();
}
#endif

#endif
