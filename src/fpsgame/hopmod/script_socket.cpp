/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "script_socket.hpp"
#include <boost/bind.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include <microhttpd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class script_socket_server
{
public:
    script_socket_server(unsigned short port,cubescript::domain * aDomain)
     :m_script_domain(aDomain)
    {
        m_daemon = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_PEDANTIC_CHECKS,
            port,
            script_socket_server::http_accept_callback,this,
            script_socket_server::http_access_callback,this,
            MHD_OPTION_NOTIFY_COMPLETED,
            script_socket_server::http_completed_callback,this,
            MHD_OPTION_END);
    }
    
    ~script_socket_server()
    {
        MHD_stop_daemon(m_daemon);
    }
    
    void run()
    {
        MHD_run(m_daemon);
    }
private:
    static int http_accept_callback(void *cls,const struct sockaddr * addr,socklen_t addrlen)
    {
        sockaddr_in * peer_addr = (sockaddr_in *)addr;
        char * ip = inet_ntoa(peer_addr->sin_addr);
        
        if(htonl(peer_addr->sin_addr.s_addr) != INADDR_LOOPBACK)
        {
            std::cerr<<"Rejected script_socket connection from "<<ip<<std::endl;
            return MHD_NO;
        }
        
        return MHD_YES;
    }
    
    static int http_access_callback(void *cls,MHD_Connection * connection,const char *url,
        const char *method, const char *version, const char *upload_data,
        unsigned int *upload_data_size,void **con_cls)
    {
        std::string * code_buffer = (std::string *)*con_cls;
        
        const char * clen = MHD_lookup_connection_value(connection,
            MHD_HEADER_KIND,"content-length");
        if(!clen) return MHD_NO;
        
        if(!code_buffer)
        {
            bool put_method = !strcmp(method,"POST");
            const char * ctype = MHD_lookup_connection_value(connection,
                MHD_HEADER_KIND,"content-type");
            bool cubescript_type = ctype && !strcmp(ctype,"text/cubescript");
            bool serverexec = !strcmp(url,"/serverexec");
            
            if(put_method && cubescript_type && serverexec)
            {
                code_buffer = new std::string();
                code_buffer->reserve(512);
                *con_cls = code_buffer;
            }
            else return MHD_NO;
        }
        
        if(upload_data)
        {
            if(*upload_data_size)
            {
                code_buffer->append(upload_data,*upload_data_size);
            }
            else if(code_buffer->length() >= atoi(clen))
            {
                script_socket_server * server = (script_socket_server *)cls;
                
                std::string responseBody;
                int status = server->execute_request(*code_buffer,responseBody);
                
                MHD_Response * response = MHD_create_response_from_data(
                    responseBody.length(),(void *)responseBody.c_str(),
                    MHD_NO,MHD_YES);
                MHD_queue_response(connection,status,response);
                MHD_destroy_response(response);
            }
            
            *upload_data_size=0;
        }
        
        return MHD_YES;
    }
    
    static void http_completed_callback(void *cls, struct MHD_Connectionconnection * connection,
        void **con_cls, enum MHD_RequestTerminationCode toe)
    {
        std::string * code_buffer = (std::string *) *con_cls;
        *con_cls=NULL;
        delete code_buffer;
    }
    
    int execute_request(const std::string & code,std::string & response)
    {
        int status = 400;
        std::stringstream output;
        
        try
        {  
            output << cubescript::exec_block(code,m_script_domain);
            status = 200;
        }
        catch(cubescript::error_context * error)
        {
            output<<cubescript::format_error_report(error)<<std::endl;
            delete error;
        }
        /*catch(const cubescript::script_error<cubescript::symbol_error> & e)
            {output<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<": "<<e.get_id()<<std::endl;}
        catch(const cubescript::script_error<cubescript::error_key> & e)
            {output<<"error in script file "<<e.get_filename()<<":"<<e.get_linenumber()<<": "<<e.what()<<" in expression \""<<e.get_expr()<<"\""<<std::endl;}
        catch(const cubescript::expr_error<cubescript::error_key> & e)
            {output<<"cubescript error: "<<e.what()<<" in expression \""<<e.get_expr()<<"\"."<<std::endl;}
        catch(const cubescript::symbol_error & e)
            {output<<"cubescript error: "<<e.what()<<": "<<e.get_id()<<std::endl;}
        catch(const cubescript::error_key & e)
            {output<<"cubescript error: "<<e.what()<<std::endl;}*/
        
        response = output.str();
        return !response.length() && status == 200 ? 204 : status;
    }
    
    MHD_Daemon * m_daemon;
    cubescript::domain * m_script_domain;
};

script_socket_service::script_socket_service()
{
    
}

script_socket_service::~script_socket_service()
{
    
}

void script_socket_service::register_function(cubescript::domain * aDomain)
{
    cubescript::symbol * create_function =
    new cubescript::function1<void,const std::string &>(
        boost::bind(&script_socket_service::create_script_socket_server,this,_1,aDomain));
    aDomain->register_symbol("script_socket_server",create_function,cubescript::domain::ADOPT_SYMBOL);
}

void script_socket_service::run()
{
    for(std::list<script_socket_server *>::iterator it = m_servers.begin();
        it != m_servers.end(); ++it)
        (*it)->run();
}

void script_socket_service::shutdown()
{
    for(std::list<script_socket_server *>::iterator it = m_servers.begin();
    it != m_servers.end(); ++it)
        delete *it;
    m_servers.clear();
}

void script_socket_service::create_script_socket_server(const std::string & port_str,cubescript::domain * aDomain)
{
    unsigned short port = cubescript::parse_type<unsigned short>(port_str);
    script_socket_server * server=new script_socket_server(port,aDomain);
    m_servers.push_back(server);
}
