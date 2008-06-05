
#ifndef SHARED_DOMAIN_HPP
#define SHARED_DOMAIN_HPP

#include "cubescript.hpp"
#include "transport.hpp"

namespace cubescript{
namespace net{

class shared_domain:public domain
{
public:
    class remote_symbol:public symbol
    {
    public:
        remote_symbol(std::string,transport *);
        std::string apply(std::list<std::string> & args,domain * d);
        void async_apply(std::list<std::string> & args,domain * d,apply_handler_t handler);
        void async_value(value_handler_t handler)const;
        std::string value()const;
    private:
        void async_apply_reply(apply_handler_t,const asio::error_code,const void *,int)const;
        void async_value_reply(value_handler_t,const asio::error_code,const void *,int)const;
        
        std::string m_name;
        transport * m_transport;
    };
    
    shared_domain(transport *);
private:    
    void process_peer_request(const transport::packet &,transport &);
    void_t register_rsymbol(std::string);
    std::string echo(std::string);
    
    domain m_protocol;
    transport * m_transport;
};

} //namespace net
} //namespace cubescript


#endif
