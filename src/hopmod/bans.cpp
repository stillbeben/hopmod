#include "bans.hpp"
#include <assert.h>

/**
    @brief Insert network address into the permanent ban set.

    This will override existing bans matching the given network address.
*/
void banned_networks::set_permanent_ban(const netmask & ipmask)
{
    unset_ban(ipmask);
    m_index.insert(ipmask);
}

template<typename ConstIterator>
void set_permanent_bans(ConstIterator start,ConstIterator end)
{
    for(ConstIterator i = start; i != end; ++i)
        set_permanent_ban(*i);
}

/**
    @brief Insert network address into the temporary ban set.
    
    This will override existing bans matching the given network address.
*/
void banned_networks::set_temporary_ban(const netmask & ipmask)
{
    unset_ban(ipmask);
    m_index.insert(ipmask);
    m_tmp_index.insert(ipmask);
}

template<typename ConstIterator>
void set_temporary_bans(ConstIterator start,ConstIterator end)
{
    for(ConstIterator i = start; i != end; ++i)
        set_temporary_ban(*i);
}

/**
    @brief Clear temporary bans.
*/
void banned_networks::unset_temporary_bans()
{
    std::set<netmask> permbans;
    get_permanent_banset(permbans, permbans.begin()); 
    m_index = permbans;
    m_tmp_index.clear();
}

/**
    @brief Remove a network address from the ban set.
*/
int banned_networks::unset_ban(const netmask & ipmask)
{
    int count = 0;
    while(m_index.erase(ipmask)) count++;
    while(m_tmp_index.erase(ipmask));
    return count;
}

/**
    @brief Search for a matched network address in either the temporary or permanent ban sets.
*/
bool banned_networks::is_banned(const netmask & ipmask)const
{
    return m_index.find(ipmask) != m_index.end();
}

/**
    @brief Return the set of permanent banned network addresses.
*/
std::vector<netmask> banned_networks::get_permanent_bans()const
{
    std::vector<netmask> permbans;
    get_permanent_banset(permbans, permbans.begin());
    return permbans;
}

/**
    @brief Return the set of temporary banned network addresses.
*/
std::vector<netmask> banned_networks::get_temporary_bans()const
{
    return std::vector<netmask>(m_tmp_index.begin(), m_tmp_index.end());
}

timedbans_service::entry::entry(const netmask & a,int e)
 :addr(a),expire(e)
{

}

bool timedbans_service::entry::operator<(const entry & y)const
{
    return expire > y.expire;
}
    
timedbans_service::timedbans_service(banned_networks & bans)
 :m_bans(bans),m_last_update(-1)
{
    
}

void timedbans_service::set_ban(const netmask & addr,int secs)
{
    assert(m_last_update!=-1 && secs >= 0);
    m_bans.set_temporary_ban(addr);
    m_ban_times.push(entry(addr,m_last_update+(secs*1000)));
}

void timedbans_service::update(int time)
{
    m_last_update = time;
    if(m_ban_times.empty()) return;
    else if(time >= m_ban_times.top().expire)
    {
        m_bans.unset_ban(m_ban_times.top().addr);
        m_ban_times.pop();
        update(time);
    }
}

void timedbans_service::clear()
{
    while(!m_ban_times.empty())
    {
        m_bans.unset_ban(m_ban_times.top().addr);
        m_ban_times.pop();
    }
}
