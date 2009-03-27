#ifndef HOPMOD_GEOIP_HPP
#define HOPMOD_GEOIP_HPP

#define GEOIP_NO_RESULT ""

bool geoip_supported();
bool load_geoip_database(const char *);
const char * ip_to_country(const char *);
const char * ip_to_country_code(const char *);

#endif
