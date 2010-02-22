#include "netmask.hpp"

#ifndef WIN32
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <bitset>
#include <typeinfo>

#include <iostream>

netmask::netmask()
 :m_prefix(0),m_mask(0xFFFFFFFF)
{
    
}

netmask::netmask(addr_t ip)
 :m_prefix(ntohl(ip)), m_mask(0xFFFFFFFF)
{
    
}

netmask::netmask(addr_t ip,int bits)
 :m_prefix(ntohl(ip))
{
    assert(bits > 0 && bits < 33);
    m_mask = (unsigned long)(pow(2, bits)-1) << (32 - bits);
    m_prefix &= m_mask;
}

/**
    @brief Parse a CIDR notation string to construct a netmask object.
    
    std::bad_cast is thrown on parsing failure.
*/
netmask netmask::make(const char * str)
{
    char str_copy[19];
    char * str_copy_p = str_copy;
    const char * tokens[5] = {str_copy,"0","0","0",NULL};
    const char ** current_token = tokens;
    
    for(; *str && str_copy_p < str_copy + sizeof(str_copy) - 1; ++str, ++str_copy_p)
    {
        char c = *str;
        bool new_token = false;
        
        if(c == '.')
        {
            if(current_token == &tokens[3]) throw std::bad_cast();
            
            ++current_token;
            new_token = true;
        }
        else if(c == '/')
        {
            if( (current_token == &tokens[0] && 
                str_copy_p == &str_copy[0]) ||
                current_token == &tokens[4] ) throw std::bad_cast();
            
            current_token = &tokens[4];
            new_token = true;
        }
        
        if(new_token)
        {
            if(*(str+1)=='\0') throw;
            *current_token = str_copy_p + 1;
            c = '\0';
        }
        
        if((c != '\0' && c < '0') || c > '9') throw std::bad_cast();
        
        *str_copy_p = c;
    }
    
    // str is too big, probably caused by leading zeros.
    if(*str && str_copy_p == str_copy + sizeof(str_copy) - 1) throw std::bad_cast();
    
    *str_copy_p = '\0';
    
    unsigned long a = atoi(tokens[0]);
    unsigned long b = atoi(tokens[1]);
    unsigned long c = atoi(tokens[2]);
    unsigned long d = atoi(tokens[3]);
    if( a > 255 || b > 255 || c > 255 || d > 255) throw std::bad_cast();
    
    unsigned long prefix = (a << 24) | (b << 16) | (c << 8) | d;
    int maskbits = tokens[4] ? atoi(tokens[4]) : 32 ;
    if(maskbits > 32 || maskbits == 0) throw std::bad_cast();
    
    return netmask(htonl(prefix),maskbits);
}

bool netmask::operator==(addr_t ip)const
{
    return (ntohl(ip) & m_mask) == m_prefix;
}

bool netmask::operator!=(addr_t ip)const
{
    return !operator==(ip);
}

bool netmask::operator==(const netmask & x)const
{
    return m_prefix == (m_mask & x.m_prefix);
}

bool netmask::operator!=(const netmask & x)const
{
    return !operator==(x);
}

bool netmask::operator<(const netmask & x)const
{
    return m_prefix < (m_mask & x.m_prefix);
}

std::string netmask::to_string()const
{
    std::bitset<32> maskbits(m_mask);
    
    std::ostringstream out;
    out <<(int)(m_prefix >> 24)          <<"."
        <<(int)(m_prefix >> 16 & 0xff)   <<"."
        <<(int)(m_prefix >> 8  & 0xff)   <<"."
        <<(int)(m_prefix       & 0xff);
    
    if(maskbits.count() < 32) out<<"/"<<maskbits.count();
    
    return out.str();
}

std::istream & operator>>(std::istream & in, netmask & ipmask)
{
    std::string str;
    in >> str;
    ipmask = netmask::make(str.c_str());
    return in;
}
