#include <lua.hpp>
#include <unistd.h>
#include <errno.h>

namespace lua{

int getcwd(lua_State * L)
{
    char buffer[512];
    if(!::getcwd(buffer, sizeof(buffer)))
        luaL_error(L, "getcwd() failed with errno %i", errno);
    lua_pushstring(L, buffer);
    return 1;
}

} //namespace lua

