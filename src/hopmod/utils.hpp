#ifndef HOPMOD_UTILS_HPP
#define HOPMOD_UTILS_HPP

#include <time.h>
#include <string>

std::string resolve_hostname(const char *);

std::string concol(int code, const std::string & msg);
std::string green(const std::string & msg);
std::string info(const std::string & msg);
std::string err(const std::string & msg);
std::string grey(const std::string & msg);
std::string magenta(const std::string & msg);
std::string orange(const std::string & msg);
std::string gameplay(const std::string & msg);
std::string red(const std::string & msg);
std::string blue(const std::string & msg);
std::string yellow(const std::string & msg);

int secs(int ms);
int mins(int ms);

class timer
{
public:
    typedef unsigned time_diff_t;
    timer();
    time_diff_t usec_elapsed()const;
private:
    timespec m_start;
};

namespace lua{
namespace crypto{

int genkeypair(lua_State *);
int genchallenge(lua_State *);
int checkchallenge(lua_State *);
int freechalanswer(lua_State *);

} //namespace crypto
} //namespace lua

std::string md5sum(const std::string & input);

#endif
