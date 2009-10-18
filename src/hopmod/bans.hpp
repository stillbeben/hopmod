/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_BANS_HPP
#define HOPMOD_BANS_HPP

#include "netmask.hpp"
#include <set>
#include <vector>
#include <queue>
#include <algorithm>

/**
    @brief Stores a set of banned network addresses.
*/
class banned_networks
{
public:
    void set_permanent_ban(const netmask & ipmask);
    void set_temporary_ban(const netmask & ipmask);
    
    template<typename ConstIterator>
    void set_permanent_bans(ConstIterator start,ConstIterator end)
    {
        for(ConstIterator i = start; i != end; ++i)
            set_permanent_ban(*i);
    }
    
    template<typename ConstIterator>
    void set_temporary_bans(ConstIterator start,ConstIterator end)
    {
        for(ConstIterator i = start; i != end; ++i)
            set_temporary_ban(*i);
    }
    
    void unset_temporary_bans();
    int unset_ban(const netmask & ipmask);
    
    bool is_banned(const netmask & ipmask)const;
    std::vector<netmask> get_permanent_bans()const;
    std::vector<netmask> get_temporary_bans()const;
private:
    template<typename Container>
    void get_permanent_banset(Container & outputContainer,typename Container::iterator beginOutput)const
    {
        // the permanent ban set is the complement set of m_tmp_index
        std::set_difference(m_index.begin(), m_index.end(), 
            m_tmp_index.begin(), m_tmp_index.end(), 
            std::inserter(outputContainer, beginOutput));
    }
    
    std::set<netmask> m_index;
    std::set<netmask> m_tmp_index;
};

class timedbans_service
{
public:
    timedbans_service(banned_networks & bans);
    void set_ban(const netmask & addr,int secs);
    void update(int time);
    void clear();
private:
    banned_networks & m_bans;
    
    struct entry
    {
        netmask addr;
        int expire;
        entry(const netmask & a,int e);
        bool operator<(const entry & y)const;
    };
    
    std::priority_queue<entry> m_ban_times;
    int m_last_update;
};

#endif
