
#include "shared_domain.hpp"
#include <functional>
#include <boost/bind.hpp>

namespace cubescript{
namespace net{

shared_domain::shared_domain(transport * t)
{
    m_transport=t;
    m_transport->set_request_reactor(boost::bind(&shared_domain::process_peer_request,this,_1,_2));
    
    static function1<std::string,std::string> func_echo(std::bind1st(std::mem_fun(&shared_domain::echo),this));
    m_protocol.register_symbol("echo",&func_echo);
    
    static function1<void_t,std::string> func_register(std::bind1st(std::mem_fun(&shared_domain::register_rsymbol),this));
    m_protocol.register_symbol("register",&func_register);
    
    //m_protocol.register_symbol("apply",);
    //m_protocol.register_symbol("value",);
}

void_t shared_domain::register_rsymbol(std::string name)
{
    remote_symbol * rs=new remote_symbol(name,m_transport);
    register_symbol(name,rs);
    adopt_symbol(rs);
    return void_t();
}

std::string shared_domain::echo(std::string content)
{
    return content;
}

void shared_domain::process_peer_request(const transport::packet & p,transport & t)
{
    
}

shared_domain::remote_symbol::remote_symbol(std::string name,transport * t)
{
    m_name=name;
    m_transport=t;
}

std::string shared_domain::remote_symbol::apply(std::list<std::string> & args,domain * d)
{
    throw runtime_error("runtime.symbol.unavailable.async_only");
}

void shared_domain::remote_symbol::async_apply(std::list<std::string> & args,domain * d,apply_handler_t handler)
{
    std::stringstream code;
    code<<"apply "<<m_name<<" ["<<print_type(args)<<"]\r\n";
    //TODO check code size
    m_transport->async_send_request(code.str().c_str(),code.str().length(),
        boost::bind(boost::mem_fn(&remote_symbol::async_apply_reply),this,handler,_1,_2,_3));
}

void shared_domain::remote_symbol::async_apply_reply(apply_handler_t handler,const asio::error_code ec,const void * payload,int payload_size)const
{
    
}

void shared_domain::remote_symbol::async_value(value_handler_t handler)const
{
    std::stringstream code;
    code<<"value "<<m_name<<"\r\n";
    //TODO check code size
    m_transport->async_send_request(code.str().c_str(),code.str().length(),
        boost::bind(boost::mem_fn(&remote_symbol::async_value_reply),this,handler,_1,_2,_3));
}

void shared_domain::remote_symbol::async_value_reply(value_handler_t handler,const asio::error_code ec,const void * payload,int payload_size)const
{
    
}

std::string shared_domain::remote_symbol::value()const
{
    throw runtime_error("runtime.symbol.unavailable.async_only");
}

} //namespace net
} //namespace cubescript
