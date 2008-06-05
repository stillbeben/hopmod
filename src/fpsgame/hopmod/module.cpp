
#include "module.hpp"

namespace cubescript
{
    
module_domain_accessor::module_domain_accessor(domain * aDomain)
 :m_domain(aDomain)
{
    
}

void module_domain_accessor::register_symbol(const std::string & id,symbol * object)
{
    m_domain->register_symbol(id,object);
}

} //namespace cubescript
