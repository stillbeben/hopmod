#include "script_bindings.hpp"
#include "cube.h"
#include <fungu/script.hpp>

using namespace fungu;

void register_server_script_bindings(script::env & env)
{
    script::bind_global_func<void (const char *)>(server::sendservmsg, FUNGU_OBJECT_ID("msg"), env);
}
