/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_NETMASK_HPP
#define HOPMOD_NETMASK_HPP

#include <string>
#include <istream>

/**
    @brief Represent an IPv4 address prefix.
*/
class netmask
{
public:
    typedef unsigned long addr_t;
    
    netmask();
    netmask(addr_t ip);
    netmask(addr_t ip,int bits);
    
    static netmask make(const char * str);
    
    bool operator==(addr_t ip)const;
    bool operator!=(addr_t ip)const;
    bool operator==(const netmask &)const;
    bool operator!=(const netmask &)const;
    bool operator<(const netmask &)const;
    bool operator<=(const netmask &)const;

    std::string to_string()const;
private:
    unsigned long m_prefix;
    unsigned long m_mask;
};

std::istream & operator>>(std::istream & in, netmask & ipmask);

#endif
