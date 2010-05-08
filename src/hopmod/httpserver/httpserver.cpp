#include <fungu/script.hpp>
#include <fungu/net/http/connection.hpp>
#include <fungu/net/http/request_line.hpp>
#include <fungu/net/http/header.hpp>
#include <fungu/net/http/info.hpp>
#include <fungu/net/http/response.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <cstdio>
#include <iostream>

#include "directory_resource.hpp"
#include "proxy_resource.hpp"
#include "filesystem_resource.hpp"
#include "serverexec.hpp"
#include "../utils.hpp"

using namespace fungu;
using namespace boost::asio;
using namespace boost::system;

boost::asio::io_service & get_main_io_service();
void setup_ext_to_ct_map();
void restart_http_server();
void stop_http_server();

static void wait_next_accept(ip::tcp::acceptor & listener);

static ip::tcp::acceptor * server_acceptor = NULL;
static proxy_resource root_resource;
static serverexec_resource serverexec;

static std::string listener_hostname;
static std::string listener_port;

static void cleanup_client_connection(http::server::client_connection * client)
{
    delete client;
}

static void accept_handler(ip::tcp::acceptor & listener, http::server::client_connection * client, const error_code & error)
{    
    if(error)
    {
        delete client;
        
        if(error.value() != error::operation_aborted) 
        {
            std::cerr<<"Error in HTTP server accept handler: "<<error.message()<<std::endl;
            restart_http_server();
        }
        
        return;
    }
    
    http::server::request::create(*client, root_resource, boost::bind(cleanup_client_connection, client));
    
    wait_next_accept(listener);
}

static void wait_next_accept(ip::tcp::acceptor & listener)
{
    http::server::client_connection * client = new http::server::client_connection(listener.get_io_service());
    listener.async_accept(*client, boost::bind(accept_handler, boost::ref(listener), client, _1));
}

template<typename Class>
void delete_object(Class * object)
{
    delete object;
}

void setup_default_root()
{
    directory_resource * root = new directory_resource;
    root->add_resource(serverexec, "serverexec");
    root_resource.set_resource(root, boost::bind(delete_object<directory_resource>, root));
}

void start_http_server(const char * ip, const char * port)
{
    setup_ext_to_ct_map();
    http::register_standard_headers();
    
    setup_default_root();
    
    boost::system::error_code ec;
    
    server_acceptor = new ip::tcp::acceptor(get_main_io_service());
    
    server_acceptor->open(ip::tcp::v4(), ec);
    if(ec)
    {
        std::cerr<<"start_http_server("<<ip<<", "<<port<<") failed on socket open: "<<ec.message()<<std::endl;
        delete server_acceptor;
        server_acceptor = NULL;
        return;
    }
    
    server_acceptor->set_option(socket_base::reuse_address(true));
    
    server_acceptor->bind(ip::tcp::endpoint(ip::address_v4::from_string(ip), atoi(port)), ec);
    if(ec)
    {
        std::cerr<<"start_http_server("<<ip<<", "<<port<<") failed on socket bind: "<<ec.message()<<std::endl;
        delete server_acceptor;
        server_acceptor = NULL;
        return;
    }
    
    server_acceptor->listen();
    
    wait_next_accept(*server_acceptor);
    
    info_file("log/sauer_server_http.port", "%s\n", port);
    std::cout<<"HTTP server listening on TCP "<<ip<<":"<<port<<std::endl;
    
    listener_hostname = ip;
    listener_port = port;
}

void restart_http_server()
{
    std::cout<<"Restarting http server"<<std::endl;
    stop_http_server();
    start_http_server(listener_hostname.c_str(), listener_port.c_str());
}

void stop_http_server()
{
    delete server_acceptor;
    server_acceptor = NULL;
}

proxy_resource & get_root_resource()
{
    return root_resource;
}
