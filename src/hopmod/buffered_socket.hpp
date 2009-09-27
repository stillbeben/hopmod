#ifndef HOPMOD_BUFFERED_SOCKET_HPP
#define HOPMOD_BUFFERED_SOCKET_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace boost{
namespace asio{

template<typename SocketClass>
class buffered_input_socket : public SocketClass
{
public:
    buffered_input_socket(boost::asio::io_service & service)
        :SocketClass(service)
    {
        
    }
    
    template<typename ReadHandler>
    void async_read_until(char delim, ReadHandler handler)
    {
        boost::asio::async_read_until(*this, m_read, delim, handler);
    }
    
    template<typename ReadHandler>
    void async_read_until(const std::string & delim, ReadHandler handler)
    {
        boost::asio::async_read_until(*this, m_read, delim, handler);
    }
    
    template<typename ReadHandler>
    void async_read(const std::size_t readSize, ReadHandler handler)
    {
        if(readSize < m_read.size())
        {
            SocketClass * super = this;
            super->get_io_service().post(boost::bind(handler, boost::system::error_code(), readSize));
        }
        else
        {
            boost::asio::async_read(*this, m_read, boost::asio::transfer_at_least(readSize - m_read.size()), handler);
        }
    }
    
    const boost::asio::streambuf & read_buffer()const
    {
        return m_read;
    }
    
    boost::asio::streambuf & read_buffer()
    {
        return m_read;
    }
    
private:
    boost::asio::streambuf m_read;
};

} //namespace asio
} //namespace boost

#endif
