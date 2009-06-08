#include <string>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
}

std::string resolve_hostname(const char * hostname)
{
    hostent * result=gethostbyname(hostname);
    if( result && 
        result->h_addrtype==AF_INET && 
        result->h_length==4 && 
        result->h_addr_list[0] )
      return inet_ntoa(to_in_addr(*((in_addr_t *)result->h_addr_list[0])));
    else return "0.0.0.0";
}

std::string concol(int code, const std::string & msg)
{
    std::string output;
    output.reserve(msg.length() + 6);
    output.append("\fs\f");
    output.append(1, '0' + code);
    output.append(msg);
    output.append("\fr");
    return output;
}

std::string green(const std::string & msg){return concol(0, msg);}
std::string info(const std::string & msg){return concol(1, msg);}
std::string err(const std::string & msg){return concol(3, msg);}
std::string grey(const std::string & msg){return concol(4, msg);}
std::string magenta(const std::string & msg){return concol(5, msg);}
std::string orange(const std::string & msg){return concol(6, msg);}
std::string gameplay(const std::string & msg){return concol(2, msg);}
std::string red(const std::string & msg){return concol(3, msg);}
std::string blue(const std::string & msg){return concol(1, msg);}
std::string yellow(const std::string & msg){return concol(2, msg);}

int secs(int ms){return ms * 1000;}
int mins(int ms){return ms * 60000;}
