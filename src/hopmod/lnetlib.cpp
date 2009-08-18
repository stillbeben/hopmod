#include "lnetlib.hpp"
#include <boost/asio.hpp>
using namespace boost::asio;
#include "main_io_service.hpp"
#include "scripting.hpp"
#include <netinet/in.h> //byte ordering functions

struct lnetlib_buffer
{
    char * start;
    char * end;
    char * produced;
    char * consumed;
};

static const char * TCP_ACCEPTOR_MT = "lnetlib_tcp_acceptor";
static const char * TCP_BASIC_SOCKET_MT = "lnetlib_tcp_basic_socket";
static const char * TCP_CLIENT_SOCKET_MT = "lnetlib_tcp_client_socket";
static const char * BUFFER_MT = "lnetlib_buffer";

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
    const char * hostname = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
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

int acceptor_set_option(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option(luaL_checkint(L, 3));
        acceptor->set_option(option);
    }
    else if(!strcmp(optname, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option(luaL_checkint(L, 3));
        acceptor->set_option(option);
    }
    
    return 0;
}

int acceptor_get_option(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option;
        acceptor->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(optname, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option;
        acceptor->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    
    return 0;
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
        {"set_option", acceptor_set_option},
        {"get_option", acceptor_get_option},
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

int socket_async_send_buffer(lua_State *);

int socket_async_send(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    
    if(lua_type(L, 2) != LUA_TSTRING) return socket_async_send_buffer(L);
    
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

void async_read_handler(int functionRef, int bufferRef, const boost::system::error_code error, const size_t readsize)
{
    lua_rawgeti(lua, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(lua, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 1;
    
    if(error)
    {
        lua_pushnil(lua);
        lua_pushstring(lua, error.message().c_str());
        nargs = 2;
    }
    else
    {
        lua_rawgeti(lua, LUA_REGISTRYINDEX, bufferRef);
        luaL_unref(lua, LUA_REGISTRYINDEX, bufferRef);
        
        lnetlib_buffer * rbuf = reinterpret_cast<lnetlib_buffer *>(lua_touserdata(lua, -1));
        rbuf->produced += readsize;
    }
    
    if(lua_pcall(lua, nargs, 0, 0) != 0)
        report_script_error(lua_tostring(lua, -1));
}

int socket_async_read(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    int reqreadsize = luaL_checkinteger(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    void * bufalloc = lua_newuserdata(L, reqreadsize + (sizeof(unsigned int)*4));
    luaL_getmetatable(L, BUFFER_MT);
    lua_setmetatable(L, -2);
    int bufferRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lnetlib_buffer * rbuf = reinterpret_cast<lnetlib_buffer *>(bufalloc);
    rbuf->start = reinterpret_cast<char *>(rbuf) + (sizeof(unsigned int)*4);
    rbuf->end = rbuf->start + reqreadsize;
    rbuf->produced = rbuf->start;
    rbuf->consumed = rbuf->start;
    
    assert(reinterpret_cast<unsigned int>(rbuf) % sizeof(unsigned int) == 0);//check for correct memory alignment
    
    socket->async_receive(buffer(rbuf->start, reqreadsize), boost::bind(async_read_handler, functionRef, bufferRef, _1, _2));
    
    return 0;
}

void async_send_buffer_handler(int functionRef, const boost::system::error_code error, const size_t written)
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

int socket_async_send_buffer(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 2, BUFFER_MT));
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    size_t writesize = buf->produced - buf->consumed;
    socket->async_send(buffer(buf->consumed, writesize), boost::bind(async_send_buffer_handler, functionRef, _1, _2));
    buf->consumed += writesize;
    
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

int socket_cancel(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->cancel();
    return 0;
}

int socket_shutdown_send(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_send);
    return 0;
}

int socket_shutdown_receive(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_receive);
    return 0;
}

int socket_shutdown(lua_State * L)
{
    reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_both);
    return 0;
}

int socket_set_option(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "keep_alive"))
    {
        boost::asio::socket_base::keep_alive option(luaL_checkint(L, 3));
        socket->set_option(option);
    }
    else if(!strcmp(optname, "linger"))
    {
        boost::asio::socket_base::linger option(luaL_checkint(L, 3), luaL_checkint(L, 4));
        socket->set_option(option);
    }
    
    return 0;
}

int socket_get_option(lua_State * L)
{
    ip::tcp::socket * socket = reinterpret_cast<ip::tcp::socket *>(lua_aux_checkobject(L, 1, TCP_BASIC_SOCKET_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "keep_alive"))
    {
        boost::asio::socket_base::keep_alive option;
        socket->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(optname, "linger"))
    {
        boost::asio::socket_base::linger option;
        socket->get_option(option);
        
        lua_pushboolean(L, option.enabled());
        lua_pushinteger(L, option.timeout());
        return 2;
    }
    
    return 0;
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
        {"async_read", socket_async_read},
        {"local_endpoint", socket_local_endpoint},
        {"remote_endpoint", socket_remote_endpoint},
        {"cancel", socket_cancel},
        {"shutdown_send", socket_shutdown_send},
        {"shutdown_receive", socket_shutdown_receive},
        {"shutdown", socket_shutdown},
        {"set_option", socket_set_option},
        {"get_option", socket_get_option},
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
    
    boost::asio::ip::tcp::endpoint endpoint;
    
    try
    {
        endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port);
    }
    catch(const boost::system::system_error & error)
    {
        return luaL_argerror(L, 2, error.code().message().c_str());
    }
    
    luaL_checktype(L, 4, LUA_TFUNCTION);
    
    lua_pushvalue(L, 4);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    if(!socket->is_open()) socket->open(ip::tcp::v4());
    
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

int buffer_to_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    unsigned int len = buf->produced - buf->consumed;
    lua_pushlstring(L, buf->consumed, len);
    buf->consumed += len;
    return 1;
}

int buffer_size(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->end - buf->start);
    return 1;
}

int buffer_read_left(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->produced - buf->consumed);
    return 1;
}

int buffer_write_left(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->end - buf->produced);
    return 1;
}

int buffer_read_uint8(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed == buf->produced) return 0;
    
    unsigned char value = *buf->consumed;
    buf->consumed += 1;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_uint16(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed + 2 > buf->produced) return 0;
    
    char * data = buf->consumed;
    unsigned char hi = *data;
    unsigned char lo = *(data + 1);
    unsigned short value = (hi << 8) + lo;
    buf->consumed += 2;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_uint32(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed + 4 > buf->produced) return 0;
    
    char * data = buf->consumed;
    unsigned char b1 = *data;
    unsigned char b2 = *(data + 1);
    unsigned char b3 = *(data + 2);
    unsigned char b4 = *(data + 3);
    unsigned long value = (b1 << 24)  + (b2 << 16) + (b3 << 8) + b4;
    buf->consumed += 4;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    int len = luaL_checkint(L, 2);
    if(buf->consumed + len > buf->produced) return 0;
    lua_pushlstring(L, buf->consumed, len);
    buf->consumed += len;
    return 1;
}

int buffer_reset(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    buf->consumed = buf->start;
    buf->produced = buf->start;
    return 0;
}

int buffer_write_uint8(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced == buf->end) return 0;    
    int value = luaL_checkint(L, 2);
    *buf->produced = value;
    buf->produced++;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_uint16(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced + 2 > buf->end) return 0;
    unsigned short value = luaL_checkint(L, 2);
    *buf->produced = value >> 8;
    *(buf->produced + 1) = value & 0xFF;
    buf->produced += 2;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_uint32(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced + 4 > buf->end) return 0;    
    int value = luaL_checkint(L, 2);
    *buf->produced = value >> 24;
    *(buf->produced + 1) = (value >> 16) & 0xFF;
    *(buf->produced + 2) = (value >> 8) & 0xFF;
    *(buf->produced + 3) = value & 0xFF; 
    buf->produced++;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    size_t len;
    const char * str = luaL_checklstring(L, 2, &len);
    if(buf->produced + len > buf->end) return 0;
    strncpy(buf->produced, str, len);
    buf->produced += len;
    lua_pushboolean(L, 1);
    return 1;
}

int create_buffer(lua_State * L)
{
    int size = luaL_checkint(L, 1);
    if(size <= 0) return luaL_argerror(L, 1, "invalid size");
    
    void * bufalloc = lua_newuserdata(L, size + (sizeof(unsigned int)*4));
    luaL_getmetatable(L, BUFFER_MT);
    lua_setmetatable(L, -2);
    
    lnetlib_buffer * rbuf = reinterpret_cast<lnetlib_buffer *>(bufalloc);
    rbuf->start = reinterpret_cast<char *>(rbuf) + (sizeof(unsigned int)*4);
    rbuf->end = rbuf->start + size;
    rbuf->produced = rbuf->start;
    rbuf->consumed = rbuf->start;
    
    assert(reinterpret_cast<unsigned int>(rbuf) % sizeof(unsigned int) == 0);//check for correct memory alignment
    
    return 1;
}

void create_buffer_metatable(lua_State * L)
{
    luaL_newmetatable(L, BUFFER_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"reset", buffer_reset},
        {"to_string", buffer_to_string},
        {"size", buffer_size},
        {"read_left", buffer_read_left},
        {"read_uint8", buffer_read_uint8},
        {"read_uint16", buffer_read_uint16},
        {"read_uint32", buffer_read_uint32},
        {"read_string", buffer_read_string},
        {"write_left", buffer_write_left},
        {"write_uint8", buffer_write_uint8},
        {"write_uint16", buffer_write_uint16},
        {"write_uint32", buffer_write_uint32},
        {"write_string", buffer_write_string},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
}

void register_lnetlib()
{
    lua = get_script_env().get_lua_state();
    main_io = &get_main_io_service();
    
    create_acceptor_metatable(lua);
    create_basic_socket_metatable(lua);
    create_client_socket_metatable(lua);
    create_buffer_metatable(lua);
    
    static luaL_Reg net_funcs[] = {
        {"async_resolve", async_resolve},
        {"tcp_acceptor", create_tcp_acceptor},
        {"tcp_client", create_tcp_client},
        {"buffer", create_buffer},
        {NULL, NULL}
    };
    
    luaL_register(lua, "net", net_funcs);
}
