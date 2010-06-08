extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <GeoIP.h>
#include <GeoIPCity.h>

static GeoIP * GeoCity = NULL;

static int load_geocity_database(lua_State * L)
{
    const char * filename = luaL_checkstring(L, 1);
    if(GeoCity) GeoIP_delete(GeoCity);
    GeoCity = GeoIP_open(filename, GEOIP_MEMORY_CACHE);
    lua_pushboolean(L, GeoCity != NULL);
    return 1;
}

static int ip_to_city(lua_State * L)
{
    if(!GeoCity) return luaL_error(L, "missing GeoCity database");
	char *city = GeoIP_record_by_addr(GeoCity, luaL_checkstring(L, 1))->city;
    lua_pushstring(L, city);
    return 1;
}

namespace lua{
namespace module{

void open_geocity(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"load_database", load_geocity_database},
        {"ip_to_city", ip_to_city},
        {NULL, NULL}
    };
    
    luaL_register(L, "geocity", functions);
}

} //namespace module
} //namespace lua
