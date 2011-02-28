#ifndef _PROJECT_LUA_PCALL_HPP
#define _PROJECT_LUA_PCALL_HPP

namespace lua{

// A lua_pcall function wrapper that uses the error handler set by the set_error_handler function
int pcall(lua_State * L, int nargs, int nresults);

void set_error_handler(lua_State * L, lua_CFunction);
void unset_error_handler(lua_State * L);
bool get_error_handler(lua_State * L);
lua_CFunction get_pcall_error_function();

bool is_callable(lua_State *, int);

} //namespace lua

#endif

