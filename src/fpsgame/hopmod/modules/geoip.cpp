
#include "../module.hpp"
#include <boost/bind.hpp>
#include <GeoIP.h>
#include <string>
  
class geoip_module:public cubescript::module
{
public:
    geoip_module()
    {
        m_geoip=GeoIP_open("share/GeoIP.dat",GEOIP_STANDARD);
    }
    
    void register_symbols(cubescript::module_domain_accessor * domain)
    {
        static cubescript::function1<std::string,const std::string &> func_country(boost::bind(&geoip_module::get_country,this,_1));
        
        domain->register_symbol("country",&func_country);
    }
private:
    std::string get_country(const std::string & ipaddr)
    {
        if(!m_geoip) throw cubescript::error_key("runtime.plugin.geoip.db_closed");
        const char * country=GeoIP_country_name_by_addr(m_geoip,ipaddr.c_str());
        if(!country) return "unknown";
        return country;
    }
    
    GeoIP * m_geoip;
};

static geoip_module modinst;

cubescript::module * get_csmodule_object()
{
    return &modinst;
}
