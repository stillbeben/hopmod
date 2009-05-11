#include "masterserver_client.hpp"

// The code for masterserver_client class is mostly copied from engine/server.cpp

masterserver_client::masterserver_client(input_reactor_function func)
 :m_socket(ENET_SOCKET_NULL),
  m_input_pos(0),m_output_pos(0),
  m_input_reactor(func)
{
    
}

masterserver_client::~masterserver_client()
{
    disconnect();
}

bool masterserver_client::connect(const ENetAddress & addr)
{
    m_socket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    
    if( enet_socket_connect(m_socket, &addr) < 0 )
    {
        enet_socket_destroy(m_socket);
        m_socket = ENET_SOCKET_NULL;
        return false;
    }
    
    enet_socket_set_option(m_socket, ENET_SOCKOPT_NONBLOCK, 1);
    
    return true;
}

void masterserver_client::disconnect()
{
    if(is_connected() == false) return;
    
    enet_socket_destroy(m_socket);
    m_socket = ENET_SOCKET_NULL;
    
    m_input.setsizenodelete(0);
    m_input_pos = 0;
    
    m_output.setsizenodelete(0);
    m_output_pos = 0;
}

const ENetSocket & masterserver_client::get_socket_descriptor()const
{
    return m_socket;
}

bool masterserver_client::is_connected()const
{
    return m_socket != ENET_SOCKET_NULL;
}

bool masterserver_client::has_queued_output()const
{
    return !m_output.empty();
}

void masterserver_client::flush_input()
{
    if(m_input.length() >= m_input.capacity()) 
        m_input.reserve(4096);
    
    ENetBuffer buf;
    buf.data = &m_input[m_input.length()];
    buf.dataLength = m_input.capacity() - m_input.length();
    
    int recv = enet_socket_receive(m_socket, NULL, &buf, 1);
    
    if(recv > 0)
    {
        m_input.advance(recv);
        process_input();
    }
    else disconnect();
}

void masterserver_client::flush_output()
{
    if(m_output.empty()) return;
    
    ENetBuffer buf;
    
    buf.data = &m_output[m_output_pos];
    buf.dataLength = m_output.length() - m_output_pos;
    
    int sent = enet_socket_send(m_socket, NULL, &buf, 1);
    
    if(sent >= 0)
    {
        m_output_pos += sent;
        if(m_output_pos >= m_output.length())
        {
            m_output.setsizenodelete(0);
            m_output_pos = 0;
        }
    }
    else disconnect();
}

void masterserver_client::send_request(const char * command,const char * const * args)
{
    vector<char> cmdline;
    cmdline.reserve(4096);
    
    cmdline.put(command, strlen(command));
    
    while(*args)
    {
        cmdline.put(" ", 1);
        cmdline.put(*args, strlen(*args));
        args++;
    }
    
    cmdline.put("\n", 1);
    
    m_output.put(cmdline.getbuf(), cmdline.length());
}

void masterserver_client::process_input()
{
    if(m_input_pos >= m_input.length()) return;
    
    char * start = &m_input[m_input_pos];
    char * end = (char *)memchr(start, '\n', m_input.length() - m_input_pos);
    
    char * argv[1 + max_args];
    int argc = 0;
    
    while(end)
    {
        *end++ = '\0';
        
        char * scan_start = start;
        
        while(scan_start < end)
        {
            char * scan_end = start;
            while(scan_end < end && !isspace(*scan_end)) scan_end++;
            
            *scan_end = '\0';
            argv[argc++] = scan_start;
            
            scan_start = scan_end + 1;
        }
        
        if(m_input_reactor && argc > 0) m_input_reactor(*this, argv[0], argc - 1, argv + 1);
        
        m_input_pos = end - m_input.getbuf();
        start = end;
        end = (char *)memchr(start, '\n', m_input.length() - m_input_pos);
        argc = 0;
    }
    
    if(m_input_pos >= m_input.length())
    {
        m_input.setsizenodelete(0);
        m_input_pos = 0;
    }
}
