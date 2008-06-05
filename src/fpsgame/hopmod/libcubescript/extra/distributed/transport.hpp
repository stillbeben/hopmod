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
#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP

#include <asio.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <queue>
#include <set>

namespace cubescript{
namespace net{

/*
    Packet format
    =============
    
    0                   1                   2                   3   
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version| Flags |   Message ID  |        Payload Size           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    
    Flags field
    ===========
    
    1000    Request(1) / Reply(0)
*/

class transport
{
public:
    class packet
    {
    public:
        //typedef asio::basic_stream_socket<Protocol> io_object;
        typedef asio::ip::tcp::socket io_object;
        typedef boost::function1<void,const asio::error_code> io_complete_handler;
        
        packet();
        packet(const packet &);
        ~packet();
        
        void async_receive(io_object *,io_complete_handler);
        void async_send(io_object *,io_complete_handler);
        
        bool is_complete()const;
        bool is_request()const;
        bool is_reply()const;
        
        int get_request_id()const;
        
        const void * get_payload()const;
        int get_payload_size()const;
    protected:
        void set_request_flag();
        bool set_request_id(int);
        bool copy_payload(const void *,size_t);
    private:
        void read_header_complete(const asio::error_code,const size_t);
        void read_payload_complete(const asio::error_code,const size_t);
        void send_packet_complete(const asio::error_code,const size_t);
        
        bool    m_request;
        int     m_msg_id;
        int     m_payload_size;
        char *  m_payload;
        bool    m_complete;
        
        char    m_header_buffer[4];
        
        io_object * m_io;
        io_complete_handler m_send_complete;
        io_complete_handler m_receive_complete;
    };
    
    class request_packet;
    class reply_packet;
    class local_request;

    typedef boost::function3<void,const asio::error_code,const void *,int> reply_handler;
    typedef boost::function2<void,const packet &,transport &> request_reactor;
    typedef boost::function1<void,const asio::error_code> error_reactor;
    
    transport(asio::ip::tcp::socket *);
    ~transport();
    void async_send_request(const void *,size_t,reply_handler);
    void send_reply(const packet &,const void *,int);
    void set_request_reactor(request_reactor);
    void set_error_reactor(error_reactor);
    void close();
private:
    void enqueue_packet(boost::shared_ptr<packet>);
    void dequeue_packet(const asio::error_code);
    
    void async_receive_packet();
    void receive_packet_complete(const asio::error_code);
    
    void cancel_pending_requests();
    void cleanup_request(int);
    
    request_reactor m_request_reactor;
    error_reactor m_error_reactor;
    
    packet m_in_packet;
    
    local_request * m_requests[255];
    int m_requests_count;
    std::queue< boost::shared_ptr<packet> > m_out_packets;

    asio::ip::tcp::socket * m_socket;
};

class transport::request_packet:public transport::packet
{
public:
    request_packet(int,const void *,size_t);
};

class transport::reply_packet:public transport::packet
{
public:
    reply_packet(const transport::packet &,const void *,size_t);
};

class transport::local_request
{
public:
    local_request(boost::shared_ptr<transport::request_packet>,transport::reply_handler);
    void process_error(const asio::error_code);
    void process_reply(const transport::packet &);
private:
    boost::shared_ptr<transport::request_packet> m_packet;
    transport::reply_handler m_reply_handler;
};

} //namespace net
} //namespace cubescript

#endif
