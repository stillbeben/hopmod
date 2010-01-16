#include <fungu/net/http/response.hpp>
#include "directory_resource.hpp"
#include "filesystem_resource.hpp"
using namespace fungu;

#include <iostream>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

void report_script_error(const char *);
directory_resource & get_root_resource();

class request_wrapper
{
    static const char * MT;
public:
    request_wrapper(http::server::request & req):m_request(req){}
    
    static int create_object(lua_State * L, http::server::request & req)
    {
        new (lua_newuserdata(L, sizeof(request_wrapper))) request_wrapper(req);
        luaL_getmetatable(L, request_wrapper::MT);
        lua_setmetatable(L, -2);
        return 1;
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &request_wrapper::__gc},
            {"content_length", &request_wrapper::get_content_length},
            {"content_type", &request_wrapper::get_content_type},
            {"content_subtype", &request_wrapper::get_content_subtype},
            {"header", &request_wrapper::get_header},
            {"uri", &request_wrapper::get_uri},
            {"uri_query", &request_wrapper::get_query_string},
            {"host", &request_wrapper::get_host},
            {"async_read_content", &request_wrapper::async_read_content},
            {"client_ip", &request_wrapper::get_client_ip},
            {"content_type", &request_wrapper::get_content_type},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
    
    static request_wrapper * get(lua_State * L, int narg)
    {
        return reinterpret_cast<request_wrapper *>(luaL_checkudata(L, narg, MT));
    }
    
    static http::server::request & get_request(lua_State * L, int narg)
    {
        return get(L, narg)->m_request;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT))->~request_wrapper();
        return 0;
    }
    
    static int get_content_length(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        lua_pushinteger(L, req->m_request.get_content_length());
        return 1;
    }
    
    static int get_header(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        const char * field_name = luaL_checkstring(L, 2);
        if(req->m_request.has_header_field(field_name)) 
            lua_pushstring(L, req->m_request.get_header_field(field_name).get_value());
        else lua_pushnil(L);
        return 1;
    }
    
    static int get_uri(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        lua_pushstring(L, req->m_request.get_uri());
        return 1;
    }
    
    static int get_query_string(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        lua_pushstring(L, req->m_request.get_uri_query());
        return 1;
    }
    
    static int get_host(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        lua_pushstring(L, req->m_request.get_host());
        return 1;
    }
    
    static void read_content_complete(lua_State * L, int functionRef, stream::char_vector_sink * sink, const http::connection::error & err)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        if(err) lua_pushnil(L);
        else lua_pushlstring(L, sink->data(), sink->size());
        
        delete sink;
        
        if(lua_pcall(L, 1, 0, 0) != 0)
            report_script_error(lua_tostring(L, -1));
    }
    
    static int async_read_content(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        stream::char_vector_sink * sink = new stream::char_vector_sink(req->m_request.get_content_length());
        
        req->m_request.async_read_content(*sink, boost::bind(&request_wrapper::read_content_complete, L, functionRef, sink, _1));
        
        return 0;
    }
    
    static int get_client_ip(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        std::string ip = req->m_request.get_connection().remote_ip_string();
        lua_pushlstring(L, ip.c_str(), ip.length());
        return 1;
    }
    
    static int get_content_type(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        const_string content_type = const_string(req->m_request.get_content_type().type());
        lua_pushstring(L, content_type.copy().c_str());
        return 1;
    }
    
    static int get_content_subtype(lua_State * L)
    {
        request_wrapper * req = reinterpret_cast<request_wrapper *>(luaL_checkudata(L, 1, MT));
        const_string content_type = const_string(req->m_request.get_content_type().subtype());
        lua_pushstring(L, content_type.copy().c_str());
        return 1;
    }
    
    http::server::request & m_request;
};

const char * request_wrapper::MT = "http::server::request";

class response_wrapper
{
    static const char * MT;
public:
    response_wrapper(http::server::response * res)
     :m_response(res),m_content_length(0), m_called_set_content_length(false)
    {
        
    }
    
    static int create_object(lua_State * L)
    {
        http::server::request & req = request_wrapper::get_request(L, 1);
        int status_code = luaL_checkint(L, 2);
        new (lua_newuserdata(L, sizeof(response_wrapper))) response_wrapper(new http::server::response(req, static_cast<http::status>(status_code)));
        luaL_getmetatable(L, response_wrapper::MT);
        lua_setmetatable(L, -2);
        return 1;
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &response_wrapper::__gc},
            {"header", &response_wrapper::add_header},
            {"set_content_length", &response_wrapper::set_content_length},
            {"async_send_header", &response_wrapper::async_send_header},
            {"send_header", &response_wrapper::send_header},
            {"async_send_body", &response_wrapper::async_send_body},
            {"send_body", &response_wrapper::send_body},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT))->~response_wrapper();
        return 0;
    }
    
    static int add_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        const char * name = luaL_checkstring(L, 2);
        const char * value = luaL_checkstring(L, 3);
        res->m_response->add_header_field(name, value);
        return 0;
    }
    
    static int set_content_length(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        if(res->m_called_set_content_length) return luaL_error(L, "you have already called the set_content_length method");
        int length = luaL_checkint(L, 2);
        res->m_content_length = length;
        res->m_response->set_content_length(length);
        res->m_called_set_content_length = true;
        return 0;
    }
    
    void sent_header(lua_State * L, int functionRef, const http::connection::error & err)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        bool sent = !err;
        lua_pushboolean(L, sent);
        
        if(lua_pcall(L, 1, 0, 0) != 0)
            report_script_error(lua_tostring(L, -1));
        
        if(m_content_length == 0) m_response = NULL;
    }
    
    static int async_send_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        res->m_response->async_send_header(boost::bind(&response_wrapper::sent_header, res, L, functionRef, _1));
        
        return 0;
    }
    
    static int send_header(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        res->m_response->send_header();
        if(res->m_content_length == 0) res->m_response = NULL;
        return 0;
    }
    
    void sent_body(lua_State * L, int functionRef, const http::connection::error & err)
    {
        lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
        luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
        
        bool sent = !err;
        lua_pushboolean(L, sent);
        
        if(lua_pcall(L, 1, 0, 0) != 0)
            report_script_error(lua_tostring(L, -1));
        
        delete m_response;
        m_response = NULL;
    }
    
    static int async_send_body(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        std::size_t bodylen = 0;
        const char * body = luaL_checklstring(L, 2, &bodylen);
        
        luaL_checktype(L, 2, LUA_TFUNCTION);
        lua_pushvalue(L, 2);
        int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
        
        res->m_response->async_send_body(body, bodylen, boost::bind(&response_wrapper::sent_body, res, L, functionRef, _1));
        
        return 0;
    }
    
    static int send_body(lua_State * L)
    {
        response_wrapper * res = reinterpret_cast<response_wrapper *>(luaL_checkudata(L, 1, MT));
        res->check_response_object(L);
        
        std::size_t bodylen = 0;
        const char * body = luaL_checklstring(L, 2, &bodylen);
        res->m_response->send_body(body, bodylen);
        res->m_response = NULL; // will self delete on sent body
        return 0;
    }
    
    void check_response_object(lua_State * L)
    {
        if(!m_response) luaL_error(L, "response object has expired");
    }
    
    http::server::response * m_response;
    std::size_t m_content_length;
    unsigned int m_called_set_content_length;
};

const char * response_wrapper::MT = "http::server::response";

class resource_wrapper:public http::server::resource
{
    static const char * MT;
public:
    resource_wrapper()
     :m_lua(NULL),
      m_resolve_function(LUA_REFNIL),
      m_get_function(LUA_REFNIL),
      m_put_function(LUA_REFNIL),
      m_post_function(LUA_REFNIL),
      m_delete_function(LUA_REFNIL),
      m_self_ref(LUA_REFNIL)
    {
        
    }
    
    ~resource_wrapper()
    {
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_resolve_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_get_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_put_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_post_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_delete_function);
        luaL_unref(m_lua, LUA_REGISTRYINDEX, m_self_ref);
    }
    
    http::server::resource * resolve(const const_string & uri)
    {
        if(m_resolve_function == LUA_REFNIL) return NULL;
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_resolve_function);
        
        if(lua_pcall(m_lua, 1, 1, 0) != 0)
        {
            report_script_error(lua_tostring(m_lua, -1));
            return NULL;
        }
        else
        {
            //TODO
            return NULL;
        }
    }
    
    void get_method(http::server::request & req)
    {
        if(m_get_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_get_function);
        
        request_wrapper::create_object(m_lua, req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(m_lua, -1));
    }
    
    void put_method(http::server::request & req)
    {
        if(m_put_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_put_function);
        
        request_wrapper::create_object(m_lua, req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(m_lua, -1));
    }
    
    void post_method(http::server::request & req)
    {
        if(m_post_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_post_function);
        
        request_wrapper::create_object(m_lua, req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(m_lua, -1));
    }
    
    void delete_method(http::server::request & req)
    {
        if(m_delete_function == LUA_REFNIL)
        {
            http::server::send_response(req, http::METHOD_NOT_ALLOWED);
            return;
        }
        lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_delete_function);
        
        request_wrapper::create_object(m_lua, req);
        
        if(lua_pcall(m_lua, 1, 0, 0) != 0)
            report_script_error(lua_tostring(m_lua, -1));
    }
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &resource_wrapper::__gc},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
    }
    
    static int create_object(lua_State * L)
    {
        luaL_checktype(L, 1, LUA_TTABLE);
        
        int resolve_function = LUA_REFNIL;
        int get_function = LUA_REFNIL;
        int put_function = LUA_REFNIL;
        int post_function = LUA_REFNIL;
        int delete_function = LUA_REFNIL;
        
        lua_pushstring(L, "resolve");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) resolve_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "get");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) get_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "put");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) put_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "post");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) post_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        lua_pushstring(L, "delete");
        lua_rawget(L, -2);
        if(lua_type(L, -1) == LUA_TFUNCTION) delete_function = luaL_ref(L, LUA_REGISTRYINDEX);
        else lua_pop(L, 1);
        
        resource_wrapper * res = new (lua_newuserdata(L, sizeof(resource_wrapper))) resource_wrapper;
        
        luaL_getmetatable(L, resource_wrapper::MT);
        lua_setmetatable(L, -2);
        
        res->m_resolve_function = resolve_function;
        res->m_get_function = get_function;
        res->m_put_function = put_function;
        res->m_post_function = post_function;
        res->m_delete_function = delete_function;
        res->m_lua = L;
        
        lua_pushvalue(L, -1);
        res->m_self_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        
        return 1;
    }
    
    static int bind_to_root(lua_State * L)
    {
        const char * name = luaL_checkstring(L, 1);
        resource_wrapper * res = reinterpret_cast<resource_wrapper *>(luaL_checkudata(L, 2, MT));
        
        get_root_resource().add_resource(*res, const_string(std::string(name)));
        return 0;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<resource_wrapper *>(luaL_checkudata(L, 1, MT))->~resource_wrapper();
        return 0;
    }
    
    int m_self_ref;
    int m_resolve_function;
    int m_get_function;
    int m_put_function;
    int m_post_function;
    int m_delete_function;
    lua_State * m_lua;
};

const char * resource_wrapper::MT = "http::server::resource";

int bind_filesystem_path(lua_State * L)
{
    const char * resource_id = luaL_checkstring(L, 1);
    const char * root_path = luaL_checkstring(L, 2);
    const char * index_file = luaL_checkstring(L, 3);
    get_root_resource().add_resource(*new filesystem_resource(std::string(root_path), std::string(index_file), NULL), std::string(resource_id));
    return 0;
}

static int url_decode(lua_State * L)
{
    std::size_t input_len;
    const char * input = luaL_checklstring(L, 1, &input_len);
    char buffer[1024];
    std::size_t output_len = sizeof(buffer);
    lua_pushstring(L, http::pct_decode(input, input + input_len, buffer, &output_len));
    return 1;
}

static int url_encode(lua_State * L)
{
    std::size_t input_len;
    const char * input = luaL_checklstring(L, 1, &input_len);
    char buffer[1024];
    std::size_t output_len = sizeof(buffer);
    lua_pushstring(L, http::pct_encode(input, input + input_len, buffer, &output_len));
    return 1;
}

namespace lua{
namespace module{

void open_http_server(lua_State * L)
{
    static luaL_Reg functions[] = {
        {"resource", &resource_wrapper::create_object},
        {"bind", &resource_wrapper::bind_to_root},
        {"response", &response_wrapper::create_object},
        {"bind_filesystem_path", bind_filesystem_path},
        {"url_decode", &url_decode},
        {"url_encode", &url_encode},
        {NULL, NULL}
    };
    
    luaL_register(L, "http_server", functions);
    
    request_wrapper::register_class(L);
    response_wrapper::register_class(L);
    resource_wrapper::register_class(L);
}

} //namespace module
} //namespace lua