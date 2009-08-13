#include "lnetlib.hpp"
#include <boost/asio.hpp>
using namespace boost::asio;
#include "main_io_service.hpp"
#include "scripting.hpp"

#include <iostream>

static const char * TCP_ACCEPTOR_MT = "lnetlib_tcp_acceptor";
static const char * TCP_BASIC_SOCKET_MT = "lnetlib_tcp_basic_socket";
static const char * TCP_CLIENT_SOCKET_MT = "lnetlib_tcp_client_socket";

static lua_State * lua;
static io_service * main_io;

void * lua_aux_checkobject(lua_State * L, int narg, const char * tname)
{
    luaL_getmetatable(L, tname);
    lua_getmetatable(L, narg);
    
    while(1)
    {
        if(lua_equal(L, -1, -2)) return lua_touserdata(L, narg);
        else
        {
            lua_getmetatable(L, -1);
            if(lua_type(L, -1) == LUA_TNIL) break;
            lua_replace(L, -2);
        }
    }
    
    luaL_argerror(L, narg, "incompatible type");
    return NULL;
}

void resolve_handler(int luaFunctionCbRef, const boost::system::error_code ec, ip::tcp::resolver::iterator it)
{
    lua_rawgeti(lua, LUA_REGISTRYINDEX, luaFunctionCbRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, luaFunctionCbRef);
    
    if(!ec)
    {
        lua_newtable(lua);
        
        int count = 1;
        for(; it != ip::tcp::resolver::iterator(); ++it)
        {
            lua_pushinteger(lua, count++);
            lua_pushstring(lua, it->endpoint().address().to_string().c_str());
            lua_settable(lua, -3);
        }
        
        if(lua_pcall(lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(lua, -1));
    }
    else
    {
        lua_pushstring(lua, ec.message().c_str());
        
        if(lua_pcall(lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(lua, -1));
    }
}

int async_resolve(lua_State * L)
{
    static ip::tcp::resolver dns(*main_io);
    int argc = lua_gettop(L);
    if(argc < 2) return luaL_error(L, "missing arguments");
    const char * hostname = lua_tostring(L, 1);
    if(lua_type(L, 2) != LUA_TFUNCTION) return luaL_argerror(L, 2, "expected function");
    lua_pushvalue(L, 2);
    ip::tcp::resolver::query query(hostname, "");
    dns.async_resolve(query, boost::bind(resolve_handler, luaL_ref(L, LUA_REGISTRYINDEX), _1, _2));
    return 0;
}

int acceptor_listen(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    acceptor->listen();
    return 0;
}

int acceptor_close(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    acceptor->close();
    return 0;
}

void async_accept_handler(int luaFunctionCbRef, int socketRef, boost::system::error_code error)
{
    lua_rawgeti(lua, LUA_REGISTRYINDEX, luaFunctionCbRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, luaFunctionCbRef);
    
    lua_rawgeti(lua, LUA_REGISTRYINDEX, socketRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, socketRef);
    
    if(error)
    {
        lua_pop(lua, 1); //socket object
        lua_pushstring(lua, error.message().c_str());
        
        if(lua_pcall(lua, 2, 0, 0) != 0)
            report_script_error(lua_tostring(lua, -1));
    }
    else
    {
        if(lua_pcall(lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(lua, -1));
    }
}

int acceptor_async_accept(lua_State * L)
{
    luaL_checktype(L, 2, LUA_TFUNCTION);
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    
    ip::tcp::socket * socket = new (lua_newuserdata(L, sizeof(ip::tcp::socket))) ip::tcp::socket(*main_io);
    luaL_getmetatable(L, TCP_BASIC_SOCKET_MT);
    lua_setmetatable(L, -2);
    int socketRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushvalue(L, 2);
    acceptor->async_accept(*socket, boost::bind(async_accept_handler, luaL_ref(L, LUA_REGISTRYINDEX), socketRef, _1));
    
    return 0;
}

int acceptor_gc(lua_State * L)
{
    reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT))->~basic_socket_acceptor<ip::tcp>();
    return 0;
}

int create_tcp_acceptor(lua_State * L)
{
    const char * ip = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    
    ip::tcp::acceptor * acceptor = new (lua_newuserdata(L, sizeof(ip::tcp::acceptor))) ip::tcp::acceptor(*main_io);
    
    // Setup metatable with __gc function set before bind() so the user-data object will be destroyed if the bind op fails.
    luaL_getmetatable(L, TCP_ACCEPTOR_MT);
    lua_setmetatable(L, -2);
    
    ip::tcp::endpoint server_ep(ip::address_v4::from_string(ip), port);
    
    acceptor->open(server_ep.protocol());
    acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
    
    boost::system::error_code error;
    acceptor->bind(server_ep, error);
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        return 2;
    }
    
    // Return user-data object
    return 1;
}

void create_acceptor_metatable(lua_State * L)
{
    luaL_newmetatable(L, TCP_ACCEPTOR_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"__gc", acceptor_gc},
        {"listen", acceptor_listen},
        {"close", acceptor_close},
        {"async_accept", acceptor_async_accept},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
}

int socket_gc(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->~basic_stream_socket<ip::tcp>();
    return 0;
}

int socket_close(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->close();
    return 0;
}

void async_send_handler(int functionRef, int stringRef, const boost::system::error_code error, const size_t len)
{
    luaL_unref(lua, LUA_REGISTRYINDEX, stringRef);
    
    lua_rawgeti(lua, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, functionRef);

    if(error) lua_pushstring(lua, error.message().c_str());
    else lua_pushnil(lua);
    
    lua_pushinteger(lua, len);
    
    if(lua_pcall(lua, 2, 0, 0) != 0)
        report_script_error(lua_tostring(lua, -1));
}

int socket_async_send(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    
    if(lua_type(L, 2) != LUA_TSTRING) return luaL_argerror(L, 2, "expected string");
    luaL_checktype(L, 3, LUA_TFUNCTION);
    
    size_t stringlen;
    const char * str = lua_tolstring(L, 2, &stringlen);
    lua_pushvalue(L, -1);
    int stringRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushvalue(L, 3);
    socket->async_send(boost::asio::buffer(str, stringlen), boost::bind(async_send_handler, luaL_ref(L, LUA_REGISTRYINDEX), stringRef, _1, _2));
    
    return 0;
}

void async_read_until_handler(int functionRef, boost::asio::streambuf * buf, const boost::system::error_code error, const size_t readlen)
{    
    lua_rawgeti(lua, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, functionRef);
    
    if(!error)
    {
        lua_pushstring(lua, boost::asio::buffer_cast<const char *>(*buf->data().begin()));
        lua_pushnil(lua);
    }
    else
    {
        lua_pushnil(lua);
        lua_pushstring(lua, error.message().c_str());
    }
    
    if(lua_pcall(lua, 2, 0, 0) != 0)
        report_script_error(lua_tostring(lua, -1));
    
    delete buf;
}

int socket_async_read_until(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));

    const char * delim = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    boost::asio::streambuf * readbuf = new boost::asio::streambuf;
    
    boost::asio::async_read_until(*socket, *readbuf, delim, boost::bind(async_read_until_handler, functionRef, readbuf, _1, _2));
    
    return 0;
}


int return_endpoint(lua_State * L, const boost::asio::ip::tcp::endpoint & endpoint, const boost::system::error_code error)
{
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        return 2;
    }
    
    lua_newtable(L);
    
    lua_pushinteger(L, endpoint.port());
    lua_setfield(L, -2, "port");
    
    lua_pushstring(L, endpoint.address().to_string().c_str());
    lua_setfield(L, -2, "ip");
    
    if(endpoint.address().is_v4())
    {
        lua_pushinteger(L, endpoint.address().to_v4().to_ulong());
        lua_setfield(L, -2, "iplong");
    }

    return 1;
}

int socket_local_endpoint(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint = socket->local_endpoint(error);
    
    return return_endpoint(L, endpoint, error);
}

int socket_remote_endpoint(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint = socket->remote_endpoint(error);
    
    return return_endpoint(L, endpoint, error);
}

void create_basic_socket_metatable(lua_State * L)
{
    luaL_newmetatable(L, TCP_BASIC_SOCKET_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"__gc", socket_gc},
        {"close", socket_close},
        {"async_send", socket_async_send},
        {"async_read_until", socket_async_read_until},
        {"local_endpoint", socket_local_endpoint},
        {"remote_endpoint", socket_remote_endpoint},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
}

int create_tcp_client(lua_State * L)
{
    ip::tcp::socket * socket = new (lua_newuserdata(L, sizeof(ip::tcp::socket))) ip::tcp::socket(*main_io);
    
    // Setup metatable with __gc function set before bind() so the user-data object will be destroyed if the bind op fails.
    luaL_getmetatable(L, TCP_CLIENT_SOCKET_MT);
    lua_setmetatable(L, -2);
    
    socket->open(ip::tcp::v4());
    
    // Return user-data object
    return 1;
}

void async_connect_handler(int functionRef, const boost::system::error_code error)
{
    lua_rawgeti(lua, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 0;
    
    if(error)
    {
        lua_pushstring(lua, error.message().c_str());
        nargs = 1;
    }
    
    if(lua_pcall(lua, nargs, 0, 0) != 0)
        report_script_error(lua_tostring(lua, -1));
}

int client_async_connect(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_CLIENT_SOCKET_MT));
    
    const char * ip = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);

    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
    
    luaL_checktype(L, 4, LUA_TFUNCTION);
    
    lua_pushvalue(L, 4);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    socket->async_connect(endpoint, boost::bind(async_connect_handler, functionRef, _1));
    
    return 0;
}

void create_client_socket_metatable(lua_State * L)
{
    luaL_newmetatable(L, TCP_CLIENT_SOCKET_MT);
    
    luaL_getmetatable(L, TCP_BASIC_SOCKET_MT);
    lua_setmetatable(L, -2);
    
    static luaL_Reg funcs[] = {
        {"__gc", socket_gc},
        {"async_connect", client_async_connect},
        {NULL, NULL}
    };
    luaL_register(L, NULL, funcs);
    
    lua_setfield(L, -1, "__index");
}

void register_lnetlib()
{
    lua = get_script_env().get_lua_state();
    main_io = &get_main_io_service();
    
    create_acceptor_metatable(lua);
    create_basic_socket_metatable(lua);
    create_client_socket_metatable(lua);
    
    static luaL_Reg net_funcs[] = {
        {"async_resolve", async_resolve},
        {"tcp_acceptor", create_tcp_acceptor},
        {"tcp_client", create_tcp_client},
        {NULL, NULL}
    };
    
    luaL_register(lua, "net", net_funcs);
}
