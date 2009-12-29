#include <fungu/script.hpp>
#include <fungu/net/http/connection.hpp>
#include <fungu/net/http/request_line.hpp>
#include <fungu/net/http/header.hpp>
#include <fungu/net/http/info.hpp>
#include <fungu/net/http/response.hpp>
#include <boost/bind.hpp>
#include <cstdio>
#include <iostream>

#include "directory_resource.hpp"
#include "serverexec.hpp"

using namespace fungu;
using namespace boost::asio;
using namespace boost::system;

boost::asio::io_service & get_main_io_service();
static void wait_next_accept(ip::tcp::acceptor & listener);

static ip::tcp::acceptor * server_acceptor = NULL;
static directory_resource root_resource;
static serverexec_resource serverexec;

static void accept_handler(ip::tcp::acceptor & listener, http::server::client_connection * client, const error_code & error)
{
    if(error)
    {
        std::cerr<<"Error in accept handler: "<<error.message()<<std::endl;
        delete client;
        return;
    }
    
    http::server::request::create(*client, root_resource);
    
    wait_next_accept(listener);
}

static void wait_next_accept(ip::tcp::acceptor & listener)
{
    http::server::client_connection * client = new http::server::client_connection(listener.get_io_service());
    listener.async_accept(*client, boost::bind(accept_handler, boost::ref(listener), client, _1));
}

void start_http_server(const char * ip, const char * port)
{
    http::register_standard_headers();
    
    root_resource.add_resource(serverexec, "serverexec");
    
    server_acceptor = new ip::tcp::acceptor(get_main_io_service());
    server_acceptor->open(ip::tcp::v4());
    server_acceptor->set_option(socket_base::reuse_address(true));
    
    boost::system::error_code ec;
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
}

directory_resource & get_root_resource()
{
    return root_resource;
}

