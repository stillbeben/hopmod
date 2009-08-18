#include "hopmod.hpp"
#include "utils.hpp"

#include <string>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
}

/*
    To be deprecated (use net.async_resolve once reactor branch is merged to ng)
*/
std::string resolve_hostname(const char * hostname)
{
    hostent * result = gethostbyname(hostname);
    if( result && 
        result->h_addrtype==AF_INET && 
        result->h_length==4 && 
        result->h_addr_list[0] )
      return inet_ntoa(to_in_addr(*((in_addr_t *)result->h_addr_list[0])));
    else return "0.0.0.0";
}

std::string concol(int code, const std::string & msg)
{
    std::string output;
    output.reserve(msg.length() + 6);
    output.append("\fs\f");
    output.append(1, '0' + code);
    output.append(msg);
    output.append("\fr");
    return output;
}

std::string green(const std::string & msg){return concol(0, msg);}
std::string info(const std::string & msg){return concol(1, msg);}
std::string err(const std::string & msg){return concol(3, msg);}
std::string grey(const std::string & msg){return concol(4, msg);}
std::string magenta(const std::string & msg){return concol(5, msg);}
std::string orange(const std::string & msg){return concol(6, msg);}
std::string gameplay(const std::string & msg){return concol(2, msg);}
std::string red(const std::string & msg){return concol(3, msg);}
std::string blue(const std::string & msg){return concol(1, msg);}
std::string yellow(const std::string & msg){return concol(2, msg);}

int secs(int ms){return ms * 1000;}
int mins(int ms){return ms * 60000;}

static unsigned long usec_diff(const timespec & t1, const timespec & t2)
{
    long secs = t2.tv_sec - t1.tv_sec;
    if(secs > 0) return ((secs - 1) * 1000000) + (1000000000 - t1.tv_nsec + t2.tv_nsec)/1000;
    else return (t2.tv_nsec - t1.tv_nsec)/1000;
}

timer::timer()
{
    clock_gettime(CLOCK_MONOTONIC, &m_start);
}

timer::time_diff_t timer::usec_elapsed()const
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return usec_diff(m_start, now);
}

namespace lua{
namespace crypto{

int genkeypair(lua_State * L)
{
    vector<char> privkeyout, pubkeyout;
    uint seed[3] = { rx_bytes, totalmillis, randomMT() };
    genprivkey(seed,sizeof(seed), privkeyout, pubkeyout);
    
    lua_pushstring(L, privkeyout.getbuf());
    lua_pushstring(L, pubkeyout.getbuf());
    
    return 2;
}

int genchallenge(lua_State * L)
{
    const char * pubkeystr = luaL_checkstring(L, 1);
    void * pubkey = parsepubkey(pubkeystr);
    
    uint seed[3] = { rx_bytes, totalmillis, randomMT() };
    
    static vector<char> challenge;
    challenge.setsizenodelete(0);
    
    void * answer = genchallenge(pubkey, seed, sizeof(seed), challenge);
    
    lua_pushlightuserdata(L, answer);
    lua_pushstring(L, challenge.getbuf());
    
    freepubkey(pubkey);
    
    return 2;
}

int checkchallenge(lua_State * L)
{
    const char * answer = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TLIGHTUSERDATA);
    void * correct = lua_touserdata(L, 2);
    int check = ::checkchallenge(answer, correct);
    lua_pushboolean(L, check);
    return 1;
}

int freechalanswer(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    void * ptr = lua_touserdata(L,1);
    freechallenge(ptr);
    return 0;
}

} //namespace crypto
} //namespace lua
