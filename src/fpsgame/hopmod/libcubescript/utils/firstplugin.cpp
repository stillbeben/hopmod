
#include "plugin.hpp"
#include <boost/bind.hpp>

class myfirst_plugin:public cubescript::plugin
{
public:
    myfirst_plugin()
     :m_firstfunc(boost::bind(&myfirst_plugin::firstfunc,this))
    {
        
    }
    
    void register_symbols(cubescript::plugin_domain_accessor * domain)
    {
        domain->register_symbol("myfirstfunc",&m_firstfunc);
    }
private:
    std::string firstfunc()
    {
        return "Hello World";
    }
    
    cubescript::function0<std::string> m_firstfunc;
};

static myfirst_plugin plugin_object;
cubescript::plugin * get_csplugin_object()
{
    return &plugin_object;
}
