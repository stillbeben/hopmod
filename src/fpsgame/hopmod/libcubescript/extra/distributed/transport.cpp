/*   libcubescript - A CubeScript engine.  */
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
 *      3. The names of the contributors must not be used to endorse or promote
 *         products derived from this software without specific prior written
 *         permission.
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
#include "transport.hpp"
#include <boost/bind.hpp>

namespace cubescript{
namespace net{

transport::transport(asio::ip::tcp::socket * socket)
:m_socket(socket)
{
    for(int i=0; i<255; i++) m_requests[i]=NULL;
    async_receive_packet();
    m_requests_count=0;
}

transport::~transport()
{
    close();
}

void transport::async_send_request(const void * buffer,size_t size,reply_handler handler)
{
    if(size > 0xFF) throw;
    
    int id=1;
    for(; id<255; id++) if(m_requests[id]==NULL) break;
    if(id==255) throw;
    
    boost::shared_ptr<request_packet> packet(new request_packet(id,buffer,size));
    
    m_requests[id]=new local_request(packet,handler);
    m_requests_count++;
    
    enqueue_packet(packet);
}

void transport::send_reply(const packet & request,const void * payload,int payload_size)
{
    enqueue_packet(boost::shared_ptr<packet>(new reply_packet(request,payload,payload_size)));
}

void transport::set_request_reactor(request_reactor rr)
{
    m_request_reactor=rr;
}

void transport::set_error_reactor(error_reactor er)
{
    m_error_reactor=er;
}

void transport::close()
{
    m_socket->cancel();
    cancel_pending_requests();
    m_socket->close();
}

void transport::enqueue_packet(boost::shared_ptr<transport::packet> packet)
{
    bool start=m_out_packets.empty();
    m_out_packets.push(packet);
    if(start) m_out_packets.front()->async_send(m_socket,boost::bind(&transport::dequeue_packet,this,_1));
}

void transport::dequeue_packet(const asio::error_code)
{
    m_out_packets.pop();
    if(!m_out_packets.empty())
        m_out_packets.front()->async_send(m_socket,boost::bind(&transport::dequeue_packet,this,_1));
}

void transport::async_receive_packet()
{
    m_in_packet.async_receive(m_socket,boost::bind(&transport::receive_packet_complete,this,_1));
}

void transport::receive_packet_complete(const asio::error_code ec)
{
    if(ec)
    {
        cancel_pending_requests();
        if(m_error_reactor) m_error_reactor(ec);
        return;
    }
    
    assert( m_in_packet.is_complete() );
    
    if(m_in_packet.is_request())
    {
        if(m_request_reactor) m_request_reactor(m_in_packet,*this);
    }
    else
    {
        int id=m_in_packet.get_request_id();
        local_request * lr=m_requests[id];
        if(!lr) throw;
        lr->process_reply(m_in_packet);
        cleanup_request(id);
    }
    
    async_receive_packet();
}

void transport::cancel_pending_requests()
{
    for(int id=0; m_requests_count && id<255; id++)
    {
        if(m_requests[id])
        {
            m_requests[id]->process_error(asio::error::make_error_code(asio::error::operation_aborted));
            cleanup_request(id);
        }
    }
}

void transport::cleanup_request(int id)
{
    assert(m_requests[id]);
    
    delete m_requests[id];
    m_requests[id]=NULL;
    m_requests_count--;
}

transport::packet::packet()
{
    m_payload_size=0;
    m_payload=NULL;
    m_io=NULL;
    m_complete=false;
}

transport::packet::packet(const packet & src)
{
    m_request=src.m_request;
    m_msg_id=src.m_msg_id;
    m_payload_size=src.m_payload_size;
    m_payload=new char[m_payload_size];
    memcpy(m_payload,src.m_payload,m_payload_size);
    m_complete=src.m_complete;
}

transport::packet::~packet()
{
    if(m_payload) delete [] m_payload;
}

void transport::packet::async_receive(io_object * socket,io_complete_handler handler)
{
    if(m_io) throw;
    
    m_io=socket;
    m_complete=false;
    m_receive_complete=handler;
    
    m_io->async_receive(
        asio::buffer(m_header_buffer,sizeof(m_header_buffer)),
        boost::bind(&packet::read_header_complete,this,_1,_2));
}

void transport::packet::async_send(io_object * socket,io_complete_handler handler)
{
    if(m_io) throw;
    if(!m_complete) throw;
    
    m_io=socket;
    m_send_complete=handler;
    
    char header_bytes[4];
    
    assert( m_msg_id <= 0xFF );
    assert( m_payload_size <= 0xFFFF );
    assert( m_payload );
    
    char byte_zero=0x10; //version field
    if(m_request) byte_zero|=8;
    
    header_bytes[0]=byte_zero;
    header_bytes[1]=m_msg_id%0xFF;
    
    unsigned short * payload_size=(unsigned short *)header_bytes[2];
    *payload_size=htons(m_payload_size);
    
    std::vector<asio::const_buffer> send_data;
    send_data.reserve(2);
    send_data.push_back(asio::buffer(header_bytes,sizeof(header_bytes)));
    send_data.push_back(asio::buffer(m_payload,m_payload_size));
    
    m_io->async_send(send_data,boost::bind(&packet::send_packet_complete,this,_1,_2) );
}

bool transport::packet::is_complete()const
{
    return m_complete;
}

bool transport::packet::is_request()const
{
    return m_request;
}

bool transport::packet::is_reply()const
{
    return !m_request;
}

int transport::packet::get_request_id()const
{
    return m_msg_id;
}

const void * transport::packet::get_payload()const
{
    return m_payload;
}

int transport::packet::get_payload_size()const
{
    return m_payload_size;
}

void transport::packet::set_request_flag()
{
    m_request=true;
}

bool transport::packet::set_request_id(int id)
{
    if( id > 0xFF ) return false;
    m_msg_id=id;
    return true;
}

bool transport::packet::copy_payload(const void * payload,size_t payload_size)
{
    if( payload_size > 0xFFFF ) return false;
    m_payload=new char[payload_size];
    memcpy(m_payload,payload,payload_size);
    return true;
}

void transport::packet::read_header_complete(const asio::error_code ec,const size_t size)
{
    if(ec)
    {
        m_receive_complete(ec);
        return;
    }
    
    if( (m_header_buffer[0] & 0x10) == false ) throw;
    
    m_request=m_header_buffer[0] & 8;
    m_msg_id=m_header_buffer[1];
    m_payload_size=ntohs(*((unsigned short *)m_header_buffer[2]));
    
    if(m_payload_size==0) read_payload_complete(ec,0);
    else
    {
        m_payload=new char[m_payload_size];
        
        m_io->async_receive(
            asio::buffer(m_payload,m_payload_size),
            boost::bind(&packet::read_payload_complete,this,_1,_2));
    }
}

void transport::packet::read_payload_complete(const asio::error_code ec,const size_t size)
{
    if(!ec) m_complete=true;
    m_receive_complete(ec);
}

void transport::packet::send_packet_complete(asio::error_code ec,const size_t size)
{
    m_send_complete(ec);
}

transport::request_packet::request_packet(int req_id,const void * payload,size_t payload_size)
{
    set_request_flag();
    set_request_id(req_id);
    copy_payload(payload,payload_size);
}

transport::reply_packet::reply_packet(const transport::packet & request,const void * payload,size_t payload_size)
{
    set_request_id(request.get_request_id());
    copy_payload(payload,payload_size);
}

transport::local_request::local_request(boost::shared_ptr<transport::request_packet> packet,transport::reply_handler handler)
:m_packet(packet),m_reply_handler(handler)
{
    
}

void transport::local_request::process_error(const asio::error_code ec)
{
    m_reply_handler(ec,NULL,0);
}

void transport::local_request::process_reply(const transport::packet & reply)
{
    m_reply_handler(asio::error_code(),reply.get_payload(),reply.get_payload_size());
}

} //namespace net
} //namespace cubescript
