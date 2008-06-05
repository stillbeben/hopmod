
#ifndef HOPMOD_PLAYERID_HPP
#define HOPMOD_PLAYERID_HPP

#include <map>
#include <string>

typedef std::pair<std::string,unsigned long> playerid;

class playeridlt
{
public:
    bool operator()(const playerid & x,const playerid & y)const
    {
        return x.second < y.second || x.second == y.second && x.first < y.first;
    }
};

template<typename T,typename Alloc=std::allocator<std::pair<const playerid,T> > > 
 class player_map:public std::map<playerid,T,playeridlt,Alloc>{};

#endif
