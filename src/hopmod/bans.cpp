#include "bans.hpp"
#include <algorithm>
#include <cassert>

void banned_networks::set_ban(const netmask & ipmask)
{
    unset_ban(ipmask);
    m_index.insert(ipmask);
}

int banned_networks::unset_ban(const netmask & ipmask)
{
    int count = 0;
    while(m_index.erase(ipmask)) count++;
    return count;
}

void banned_networks::clear()
{
    m_index.clear();
}

bool banned_networks::is_banned(const netmask & ipmask)const
{
    return m_index.find(ipmask) != m_index.end();
}

std::vector<netmask> banned_networks::get_bans()const
{
    return std::vector<netmask>(m_index.begin(), m_index.end());
}

ban_times::entry::entry(const netmask & a,int e)
 :addr(a),expire(e)
{

}

bool ban_times::entry::operator<(const entry & y)const
{
    return expire > y.expire;
}
    
ban_times::ban_times(banned_networks & bans)
 :m_bans(bans),m_last_update(-1)
{
    
}

void ban_times::add(const netmask & addr, int secs)
{
    assert(m_last_update!=-1 && secs >= 0);
    m_ban_times.push( entry(addr, m_last_update + (secs*1000)) );
}

void ban_times::update(int time)
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

void ban_times::clear()
{
    while(!m_ban_times.empty()) m_ban_times.pop();
}

ban_manager::ban_manager()
 :m_ban_timer(m_temporary_bans)
{
    
}

void ban_manager::add(const netmask & address, int secs)
{
    if(secs == -1) m_permanant_bans.set_ban(address);
    else
    {
        m_temporary_bans.set_ban(address);
        m_ban_timer.add(address, secs);
    }
}

bool ban_manager::remove(const netmask & address)
{
    return (m_temporary_bans.unset_ban(address) + m_permanant_bans.unset_ban(address)) > 0;
}

void ban_manager::update(int time)
{
    m_ban_timer.update(time);
}

void ban_manager::clear_temporary_bans()
{
    m_ban_timer.clear();
    m_temporary_bans.clear();
}

std::vector<netmask> ban_manager::permanant_bans()const
{
    return m_permanant_bans.get_bans();
}

std::vector<netmask> ban_manager::temporary_bans()const
{
    return m_temporary_bans.get_bans();
}

std::vector<netmask> ban_manager::bans()const
{
    std::vector<netmask> output;
    std::vector<netmask> tempbans = m_temporary_bans.get_bans();
    std::vector<netmask> permbans = m_permanant_bans.get_bans();
    std::set_union(tempbans.begin(), tempbans.end(), 
                   permbans.begin(), permbans.end(), 
                    std::inserter(output, output.begin()) );
    return output;
}

bool ban_manager::is_banned(const netmask & address)const
{
    return m_temporary_bans.is_banned(address) || m_permanant_bans.is_banned(address);
}

