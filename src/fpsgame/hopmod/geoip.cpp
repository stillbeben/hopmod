
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

const char * get_country_by_addr(const char * ipaddr)
{
    if(!gs_geoip) throw cubescript::error_key("runtime.function.country.no_data");
    const char * country = GeoIP_country_name_by_addr(gs_geoip, ipaddr);
    if(!country) country = "unknown";
    return country;
}

const char * get_countrycode_by_addr(const char * ipaddr)
{
    if(!gs_geoip) throw cubescript::error_key("runtime.function.country.no_data");
    const char * code = GeoIP_country_code_by_addr(gs_geoip, ipaddr);
    if(!code) throw cubescript::error_key("runtime.function.countrycode.not_found");
    return code;
}

std::string get_region_by_addr(const char * ipaddr)
{
    if(!gs_geoip) throw cubescript::error_key("runtime.function.country.no_data");
    GeoIPRegion * region = GeoIP_region_by_addr(gs_geoip, ipaddr);
    if(!region) throw cubescript::error_key("runtime.function.region.not_found");
    return std::string(region->country_code) + std::string(" ") + std::string(region->region);
}

void register_geoip_functions(cubescript::domain * aDomain)
{
    static cubescript::function1<void,const char *> load_geoip_data_func(load_geoip_database);
    aDomain->register_symbol("load_geoip_data",&load_geoip_data_func);
    
    static cubescript::function1<const char *,const char *> country_func(get_country_by_addr);
    aDomain->register_symbol("country",&country_func);
    
    static cubescript::function1<const char *,const char *> countrycode_func(get_countrycode_by_addr);
    aDomain->register_symbol("countrycode",&countrycode_func);
    
    static cubescript::function1<std::string,const char *> region_func(get_region_by_addr);
    aDomain->register_symbol("region",&region_func);
}

#else
void register_geoip_functions(cubescript::domain *){}
#endif
