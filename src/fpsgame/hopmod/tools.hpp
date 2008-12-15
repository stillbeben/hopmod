
#ifndef HOPMOD_TOOLS_HPP
#define HOPMOD_TOOLS_HPP

#include <sys/types.h>
#include <vector>
#include <list>
#include <string>
#include <time.h>

template<typename T>
class assign
{
public:
    assign(T value):m_value(value){}
    void operator()(T & ref){ref=m_value;}
private:
    T m_value;
};

/**
    
*/
bool is_shell_syntax(char);

/**
    
*/
std::string shell_quote(const char *);

/**
    
*/
std::string resolve_hostname(const char *);

/**
    
*/
void kill_process(pid_t pid);

/**
    
*/
void sleepms(int ms);

/**
    
*/
pid_t spawn_daemon(const std::string & filename,const std::vector<std::string> & args,const std::string & stdoutfile,const std::string & stderrfile);

#endif
