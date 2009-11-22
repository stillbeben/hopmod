/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_BANS_HPP
#define HOPMOD_BANS_HPP

#include "netmask.hpp"
#include <set>
#include <vector>
#include <queue>

class banned_networks
{
public:
    void set_ban(const netmask & ipmask);
    template<typename ConstIterator>
    void set_bans(ConstIterator start, ConstIterator end)
    {
        for(ConstIterator i = start; i != end; ++i) set_ban(*i);
    }
    int unset_ban(const netmask & ipmask);
    void clear();
    bool is_banned(const netmask & ipmask)const;
    std::vector<netmask> get_bans()const;
private:
    std::set<netmask> m_index;
};

class ban_times
{
public:
    ban_times(banned_networks & bans);
    void add(const netmask & addr, int secs);
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

class ban_manager
{
public:
    ban_manager();
    void add(const netmask &, int secs = -1);
    bool remove(const netmask &);
    void update(int time);
    void clear_temporary_bans();
    bool is_banned(const netmask &)const;
    std::vector<netmask> permanant_bans()const;
    std::vector<netmask> temporary_bans()const;
    std::vector<netmask> bans()const;
private:
    banned_networks m_temporary_bans;
    banned_networks m_permanant_bans;
    ban_times m_ban_timer;
};

#endif
