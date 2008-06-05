
#include "plugin.hpp"

namespace cubescript
{
    
plugin_domain_accessor::plugin_domain_accessor(domain * aDomain)
 :m_domain(aDomain)
{
    
}

void plugin_domain_accessor::register_symbol(const std::string & id,symbol * object)
{
    m_domain->register_symbol(id,object);
}

} //namespace cubescript
