#include "hopmod.hpp"
#ifndef WITHOUT_GEOIP
#include <GeoIP.h>

static GeoIP * geoip = NULL;

bool geoip_supported()
{
    return true;
}

bool load_geoip_database(const char * filename)
{
    if(geoip) GeoIP_delete(geoip);
    geoip = GeoIP_open(filename, GEOIP_STANDARD | GEOIP_MEMORY_CACHE);
    return geoip;
}

const char * ip_to_country(const char * ipaddr)
{
    if(!geoip) return GEOIP_NO_RESULT;
    const char * country = GeoIP_country_name_by_addr(geoip, ipaddr); 
    return (country ? country : "");
}

const char * ip_to_country_code(const char * ipaddr)
{
    if(!geoip) return GEOIP_NO_RESULT;
    const char * code = GeoIP_country_code_by_addr(geoip, ipaddr);
    return (code ? code : "");
}

#else
bool geoip_supported(){return false;}
bool load_geoip_database(const char *){return false;}
const char * ip_to_country(const char *){return GEOIP_NO_RESULT;}
const char * ip_to_country_code(const char *){return GEOIP_NO_RESULT;}
#endif
