#ifndef HOPMOD_MASTERSERVER_CLIENT_HPP
#define HOPMOD_MASTERSERVER_CLIENT_HPP

#include "cube.h"

class masterserver_client
{
public:
    typedef void (* input_reactor_function)(masterserver_client &, const char *, int, const char * const *);
    static const int max_args = 20;
    
    masterserver_client(input_reactor_function);
    ~masterserver_client();

    bool connect(const ENetAddress & addr);
    void disconnect();
    
    const ENetSocket & get_socket_descriptor()const;
    bool is_connected()const;
    bool has_queued_output()const;

    void flush_input();
    void flush_output();
    
    void send_request(const char * command,const char * const * args);
private:
    void process_input();
    
    ENetSocket m_socket;
    vector<char> m_input;
    int m_input_pos;
    vector<char> m_output;
    int m_output_pos;
    input_reactor_function m_input_reactor;
};

#endif
