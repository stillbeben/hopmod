
#ifndef HOPMOD_BANNED_NETWORKS_DB_HPP
#define HOPMOD_BANNED_NETWORKS_DB_HPP

#include "netmask.hpp"
#include <cubescript.hpp>
#include <set>
#include <algorithm>
#include <fstream>
#include <time.h>

class banned_networks_db
{
public:
    banned_networks_db()
     :m_addban_func(boost::bind(&banned_networks_db::add_ban,this,_1,_2,true)),
      m_isbanned_func(boost::bind(&banned_networks_db::is_banned,this,_1)),
      m_clearbans_func(boost::bind(&banned_networks_db::remove_clearable_bans,this)),
      m_removeban_func(boost::bind(&banned_networks_db::remove_ban,this,_1,true)),
      m_loadbans_func(boost::bind(&banned_networks_db::load,this,_1))
    {
        
    }
    
    banned_networks_db(const banned_networks_db & src)
     : m_prefix_index(src.m_prefix_index),
       m_clearable_prefixes(src.m_clearable_prefixes),
       m_dbfilename(src.m_dbfilename),
      m_addban_func(boost::bind(&banned_networks_db::add_ban,this,_1,_2,true)),
      m_isbanned_func(boost::bind(&banned_networks_db::is_banned,this,_1)),
      m_clearbans_func(boost::bind(&banned_networks_db::remove_clearable_bans,this)),
      m_removeban_func(boost::bind(&banned_networks_db::remove_ban,this,_1,true)),
      m_loadbans_func(boost::bind(&banned_networks_db::load,this,_1))
    {
        
    }
    
    void register_script_functions(cubescript::domain * aDomain)
    {
        aDomain->register_symbol("ban", &m_addban_func);
        //aDomain->register_symbol("isbanned", &m_isbanned_func);
        aDomain->register_symbol("clearbans", &m_clearbans_func);
        aDomain->register_symbol("removeban", &m_removeban_func);
        aDomain->register_symbol("loadbans", &m_loadbans_func);
    }
    
    void add_ban(netmask ipmask, bool perm, bool commit=true)
    {
        m_prefix_index.insert(ipmask);
        if(!perm) m_clearable_prefixes.insert(ipmask);
        else m_clearable_prefixes.erase(ipmask);
        if(perm && commit) 
            append_line(std::string("ban ") + ipmask.to_string());
    }
    
    bool is_banned(netmask ipmask)const
    {
        return m_prefix_index.find(ipmask) != m_prefix_index.end();
    }
    
    void remove_clearable_bans()
    {
        std::set<netmask> difference;
        std::set_difference(m_prefix_index.begin(),m_prefix_index.end(),
            m_clearable_prefixes.begin(),m_clearable_prefixes.end(),
            std::inserter(difference,difference.begin()));
        
        m_prefix_index = difference;
        
        m_clearable_prefixes.clear();
    }
    
    void remove_ban(netmask ipmask, bool commit=true)
    {
        //only commit if matches existing perm ban
        commit = commit && m_prefix_index.count(ipmask) &&
            m_clearable_prefixes.count(ipmask) == 0;
        
        m_prefix_index.erase(ipmask);
        m_clearable_prefixes.erase(ipmask);
        if(commit) append_line(std::string("removeban ") + ipmask.to_string());
    }
    
    void load(const char * filename)
    {
        m_dbfilename = filename;
        
        cubescript::domain banconfapi;
        cubescript::function1<void,netmask> addban_func(boost::bind(&banned_networks_db::add_ban,this,_1,true,false));
        cubescript::function1<void,netmask> removeban_func(boost::bind(&banned_networks_db::remove_ban,this,_1,false));
        banconfapi.register_symbol("ban", &addban_func);
        banconfapi.register_symbol("removeban", &removeban_func);
        
        cubescript::exec_file(m_dbfilename, &banconfapi);
    }
private:
    void append_line(std::string line)const
    {
        if(m_dbfilename.length() == 0) return;
        std::ofstream file(m_dbfilename.c_str(), std::ios_base::app);
        file<<line<<std::endl;
    }
    
    std::set<netmask> m_prefix_index;
    std::set<netmask> m_clearable_prefixes;
    
    std::string m_dbfilename;
    
    cubescript::function2<void,netmask,bool> m_addban_func;
    cubescript::function1<bool,in_addr_t> m_isbanned_func;
    cubescript::function0<void> m_clearbans_func;
    cubescript::function1<void,netmask> m_removeban_func;
    cubescript::function1<void,const char *> m_loadbans_func;
};

#endif
