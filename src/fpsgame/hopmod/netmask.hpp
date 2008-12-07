
#ifndef HOPMOD_NETMASK_HPP
#define HOPMOD_NETMASK_HPP

#include <arpa/inet.h>
#include <math.h>
#include <string>
#include <sstream>
#include <bitset>

class netmask
{
public:
    netmask()
     :m_prefix(0),m_mask(0xFFFFFFFF)
    {
        
    }
    
    netmask(in_addr_t ip)
     :m_prefix(ntohl(ip)), m_mask(0xFFFFFFFF)
    {
        
    }
    
    static netmask make(const char * str)
    {
        char str_copy[19];
        char * str_copy_p = str_copy;
        char * tokens[5] = {str_copy,"0","0","0",NULL};
        char ** current_token = tokens;
        
        for(; *str && str_copy_p < str_copy + sizeof(str_copy) - 1; ++str, ++str_copy_p)
        {
            char c = *str;
            bool new_token = false;
            
            if(c == '.' )
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
            
            if(c != '\0' && c < '0' || c > '9') throw std::bad_cast();
            
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
        
        netmask result;
        result.m_prefix = (a << 24) | (b << 16) | (c << 8) | d;
        
        result.m_mask = tokens[4] ? atoi(tokens[4]) : 32 ;
        if(result.m_mask > 32 || result.m_mask == 0) throw std::bad_cast();
        result.m_mask = (unsigned long)pow(2,result.m_mask)-1 << (32 - result.m_mask);
        
        result.m_prefix &= result.m_mask;
        
        return result;
    }
    
    bool operator==(in_addr_t ip)const
    {
        return (ntohl(ip) & m_mask) == m_prefix;
    }
    
    bool operator==(const netmask & x)const
    {
        unsigned long shortest_mask = std::min(m_mask, x.m_mask);
        return (shortest_mask & m_prefix) & (shortest_mask & x.m_prefix);
    }
    
    bool operator<(const netmask & x)const
    {
        unsigned long shortest_mask = std::min(m_mask, x.m_mask);
        return (shortest_mask & m_prefix) < (shortest_mask & x.m_prefix);
    }
    
    std::string to_string()const
    {
        std::bitset<32> maskbits(m_mask);
        
        std::ostringstream out;
        out <<(m_prefix >> 24)          <<"."
            <<(m_prefix >> 16 & 0xff)   <<"."
            <<(m_prefix >> 8  & 0xff)   <<"."
            <<(m_prefix       & 0xff);
        
        if(maskbits.count() < 32) out<<"/"<<maskbits.count();
        
        return out.str();
    }
private:
    unsigned long m_prefix;
    unsigned long m_mask;
};

std::istream & operator>>(std::istream & in, netmask & ipmask)
{
    std::string str;
    in >> str;
    ipmask = netmask::make(str.c_str());
    return in;
}

#endif
