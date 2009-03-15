#ifndef HOPMOD_BANNED_NETWORKS_HPP
#define HOPMOD_BANNED_NETWORKS_HPP

#include "netmask.hpp"
#include <set>
#include <vector>
#include <algorithm>

/**
    @brief Stores a set of banned network addresses.
*/
class banned_networks
{
public:
    /**
        @brief Insert network address into the permanent ban set.
    
        This will override existing bans matching the given network address.
    */
    void set_permanent_ban(const netmask & ipmask)
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
    void set_temporary_ban(const netmask & ipmask)
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
    void unset_temporary_bans()
    {
        std::set<netmask> permbans;
        get_permanent_banset(permbans, permbans.begin()); 
        m_index = permbans;
        m_tmp_index.clear();
    }
    
    /**
        @brief Remove a network address from the ban set.
    */
    void unset_ban(const netmask & ipmask)
    {
        while(m_index.erase(ipmask));
        while(m_tmp_index.erase(ipmask));
    }
    
    /**
        @brief Search for a matched network address in either the temporary or permanent ban sets.
    */
    bool is_banned(const netmask & ipmask)const
    {
        return m_index.find(ipmask) != m_index.end();
    }
    
    /**
        @brief Return the set of permanent banned network addresses.
    */
    std::vector<netmask> get_permanent_bans()const
    {
        std::vector<netmask> permbans;
        get_permanent_banset(permbans, permbans.begin());
        return permbans;
    }
    
    /**
        @brief Return the set of temporary banned network addresses.
    */
    std::vector<netmask> get_temporary_bans()const
    {
        return std::vector<netmask>(m_tmp_index.begin(), m_tmp_index.end());
    }
    
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

#endif
