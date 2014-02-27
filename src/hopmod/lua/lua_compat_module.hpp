#ifndef _LUA_COMPAT_MODULE_HPP
#define _LUA_COMPAT_MODULE_HPP

#include <lua.hpp>

extern "C" {

void lua_compat_module_init(lua_State *L);

//make up for liblua compiled without LUA_COMPAT_MODULE. Fortunately it's just a few extra glue functions, copied directly from the source.

#if !defined(LUA_COMPAT_MODULE)

LUALIB_API void (luaL_pushmodule) (lua_State *L, const char *modname,
                                   int sizehint);
LUALIB_API void (luaL_openlib) (lua_State *L, const char *libname,
                                const luaL_Reg *l, int nup);

#define luaL_register(L,n,l)	(luaL_openlib(L,(n),(l),0))

#endif

}

#endif
