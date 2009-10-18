#include "utils.hpp"

#include <string>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
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

bool file_exists(const char * name)
{
    struct stat info;
    if(stat(name, &info)==0) return !(info.st_mode & S_IFDIR);
    else return false;
}

bool dir_exists(const char * name)
{
    struct stat info;
    if(stat(name, &info)==0) return info.st_mode & S_IFDIR;
    else return false;
}

freqlimit::freqlimit(int length)
 :m_length(length),
  m_hit(0)
{
    
}

int freqlimit::next(int time)
{
    if(time >= m_hit)
    {
        m_hit = time + m_length;
        return 0;
    }else return m_hit - time; 
}
