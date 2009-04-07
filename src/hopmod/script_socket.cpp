#include "script_socket.hpp"
#include "signals.hpp"
#include "scripting.hpp"

#ifndef WITHOUT_MICROHTTPD
#include <iostream>
#include <string>
#include <microhttpd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace fungu;

static MHD_Daemon * s_daemon = NULL;
static bool processing_request = false;
static bool sched_close = false;

static int http_accept(void *, const struct sockaddr *, socklen_t);
static int http_access(void *,MHD_Connection *,const char *,const char *, const char *,const char *,unsigned int *, void **);
static void http_completed(void *,struct MHD_Connectionconnection *,void **,enum MHD_RequestTerminationCode);

bool script_socket_supported()
{
    return true;
}

void init_script_socket()
{
    signal_shutdown.connect(close_script_socket);
}

bool open_script_socket(unsigned short port)
{
    if(s_daemon) return false;
    
    s_daemon = MHD_start_daemon(MHD_USE_DEBUG | MHD_USE_PEDANTIC_CHECKS, 
        port,
        http_accept,NULL,
        http_access,NULL,
        MHD_OPTION_NOTIFY_COMPLETED,
        http_completed,NULL,
        MHD_OPTION_END);
    
    sched_close = false;
    
    return s_daemon;
}

void run_script_socket_service()
{
    if(s_daemon)
    {
        MHD_run(s_daemon);
        
        if(sched_close && !processing_request) 
            close_script_socket();
    }
}

void close_script_socket()
{
    if(processing_request)
    {
        sched_close = true;
        return;
    }
    
    if(s_daemon) MHD_stop_daemon(s_daemon);
    s_daemon = NULL;
}

int execute_request(const std::string & request,std::string & response)
{
    int status = 400;
    std::stringstream output;
    
    try
    {
        output<<script::execute_text(request,get_script_env().get_global_scope()).to_string();
        status = 200;
    }
    catch(script::error error)
    {
        output<<error.get_error_message()<<std::endl;
    }
    catch(script::error_info * error)
    {
        output<<get_script_error_message(error)<<std::endl;
    }
    
    response = output.str();
    return !response.length() && status == 200 ? 204 : status;
}

int http_accept(void *, const struct sockaddr * addr, socklen_t addrlen)
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

int http_access(void *, MHD_Connection * connection, const char * url,
    const char * method, const char * version, const char * upload_data,
    unsigned int * upload_data_size, void ** con_cls)
{
    std::string * code_buffer = (std::string *)*con_cls;
    
    const char * clen = MHD_lookup_connection_value(connection,
        MHD_HEADER_KIND,"content-length");
    if(!clen) return MHD_NO;
    
    if(!code_buffer)
    {
        processing_request = true;
        
        bool put_method = !strcmp(method,"POST");
        const char * ctype = MHD_lookup_connection_value(connection,
            MHD_HEADER_KIND,"content-type");
        bool cubescript_type = ctype && !strcmp(ctype,"text/x-cubescript");
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
            std::string responseBody;
            int status = execute_request(*code_buffer,responseBody);
            
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

void http_completed(void *, struct MHD_Connectionconnection * connection,
    void **con_cls, enum MHD_RequestTerminationCode toe)
{
    std::string * code_buffer = (std::string *) *con_cls;
    *con_cls = NULL;
    delete code_buffer;
    processing_request = false;
}

#else
bool script_socket_supported(){return false;}
void init_script_socket(){}
bool open_script_socket(unsigned short){return false;}
void run_script_socket_service(){};
void close_script_socket(){};
#endif