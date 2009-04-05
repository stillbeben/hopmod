#include <map>
#include <string>
#include <iostream>

class playerid
{
public:
    playerid():ip(0){}
    playerid(const std::string & n,unsigned long i):name(n), ip(i){}
    
    bool operator<(const playerid & x)const
    {
        return ip < x.ip || ip == x.ip && name < x.name;
    }

    std::string name;
    unsigned long ip;
};

static std::map<playerid,int> id_map;

int get_player_id(const char * name, unsigned long ip)
{
    playerid id(name,ip);
    std::map<playerid,int>::iterator it = id_map.find(id);
    if(it == id_map.end())
    {
        int newid = id_map.size()+1;
        id_map[id] = newid;
        return newid;
    }
    return it->second;
}
