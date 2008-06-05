
#ifndef MODULE_HPP
#define MODULE_HPP

#include <cubescript.hpp>

namespace cubescript
{

/*!
    @brief Access proxy to a domain object.
    
    The purpose of this class is to give the module class a limited interface
    to the given domain object.
*/
class module_domain_accessor
{
public:
    module_domain_accessor(domain *);
    void register_symbol(const std::string &,symbol *);
private:
    domain * m_domain;
};

class module
{
public:
    virtual void register_symbols(module_domain_accessor *)=0;
};

} //namespace cubescript

#ifdef CUBESCRIPT_MODULE
extern "C"
{
    cubescript::module * get_csmodule_object();
}
#endif

#endif
