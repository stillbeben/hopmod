#include <string>
#include <netdb.h>
#include <arpa/inet.h>

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

