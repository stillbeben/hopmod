#ifndef HOPMOD_TIMED_BAN_HPP
#define HOPMOD_TIMED_BAN_HPP

#include "banned_networks.hpp"
#include <queue>

class timedbans_service
{
    struct entry
    {
        netmask addr;
        int expire;
        entry(const netmask & a,int e):addr(a),expire(e){}
        bool operator<(const entry & y)const{return expire > y.expire;}
    };
    
    banned_networks & m_bans;
    std::priority_queue<entry> m_ban_times;
    int m_last_update;
public:
    timedbans_service(banned_networks & bans)
     :m_bans(bans),m_last_update(-1)
    {
        
    }
    
    void set_ban(const netmask & addr,int secs)
    {
        assert(m_last_update!=-1 && secs >= 0);
        m_bans.set_temporary_ban(addr);
        m_ban_times.push(entry(addr,m_last_update+(secs*1000)));
    }
    
    void update(int time)
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
};

#endif
