
#include "geoip.hpp"
#include <GeoIP.h>

#ifdef USE_GEOIP
static GeoIP * gs_geoip = NULL;

void load_geoip_database(const char * filename)
{
    if(gs_geoip) GeoIP_delete(gs_geoip);
    gs_geoip=GeoIP_open(filename,GEOIP_STANDARD | GEOIP_MEMORY_CACHE);
    if(!gs_geoip)
        throw cubescript::error_key("runtime.function.load_geoip_data.open_failed");
}

std::string get_ip_to_country(const char * ipaddr)
{
    if(!gs_geoip) 
        throw cubescript::error_key("runtime.function.country.no_data");
    const char * country=GeoIP_country_name_by_addr(gs_geoip,ipaddr);
    if(!country) country="unknown";
    return country;
}

void register_geoip_functions(cubescript::domain * aDomain)
{
    static cubescript::function1<void,const char *> load_geoip_data_func(load_geoip_database);
    static cubescript::function1<std::string,const char *> country_func(get_ip_to_country);
    aDomain->register_symbol("country",&country_func);
    aDomain->register_symbol("load_geoip_data",&load_geoip_data_func);
}

#else
void register_geoip_functions(cubescript::domain *){}
#endif
