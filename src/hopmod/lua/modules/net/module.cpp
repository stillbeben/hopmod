#include <lua.hpp>
#include "../../event.hpp"
#include "tcp_socket.hpp"
#include "tcp_acceptor.hpp"
#include "ipmask.hpp"
#include "file_stream.hpp"
#include "resolver.hpp"

boost::asio::io_service & get_main_io_service();
lua::event_environment & event_listeners();

void log_error(lua_State * L, const char * event_name)
{
    event_listeners().log_error(event_name, lua_tostring(L, -1));
    lua_pop(L, 1);
}

boost::asio::io_service & get_main_io_service(lua_State *)
{
    return get_main_io_service();
}

namespace lua{
namespace module{

void open_net2(lua_State * L)
{
    lua::managed_tcp_socket::register_class(L);
    lua::tcp_acceptor::register_class(L);
    lua::ipmask::register_class(L);
    lua::ipmask_table::register_class(L);
    lua::file_stream::register_class(L);
    
    static luaL_Reg net_funcs[] = {
        {"async_resolve", lua::async_resolve},
        {"tcp_client", lua::managed_tcp_socket::create_object},
        {"tcp_acceptor", lua::tcp_acceptor::create_object},
        {"ipmask", lua::ipmask::create_object},
        {"ipmask_table", lua::ipmask_table::create_object},
        {"file_stream", lua::file_stream::create_object},
        {NULL, NULL}
    };
    
    luaL_register(L, "net", net_funcs);
    lua_pop(L, 1);
}

} //namespace module
} //namespace lua

