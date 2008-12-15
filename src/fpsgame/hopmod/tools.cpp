
#include "tools.hpp"

#include <stdarg.h>
#include <zlib.h>
#include "tools.h"

#include <iostream>
#include <sstream>
#include <cubescript/error.hpp>

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>

bool is_shell_syntax(char c)
{
    switch(c)
    {
        case '\\':case '\"':case '\'':case '$':case '&':case '>':case '<':case '*':
        case '?':case '[':case ']':case '(':case ')':case '|':case '`':case ';':
        case '#':case '^':case '!':case '~':case '\r':case '\n':case ' ':case '\t':
        case '\0': return true;
        default: return false;
    }
    return false;
}

std::string shell_quote(const char * str)
{
    std::string result;
    for(; *str; ++str)
    {
        if(is_shell_syntax(*str)) result+='\\';
        result+=*str;
    }
    return result;
}

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
}

std::string resolve_hostname(const char * hostname)
{
    hostent * result=gethostbyname(hostname);
    if( result && 
        result->h_addrtype==AF_INET && 
        result->h_length==4 && 
        result->h_addr_list[0] )
      return inet_ntoa(to_in_addr(*((in_addr_t *)result->h_addr_list[0])));
    else return "0.0.0.0";
}

void kill_process(pid_t pid)
{
    if(::kill(pid,SIGTERM)==-1)
    {
        switch(errno)
        {
            case EPERM: throw cubescript::error_key("runtime.function.kill.permission_denied");
            case ESRCH: throw cubescript::error_key("runtime.function.kill.pid_not_found");
            default: throw cubescript::error_key("runtime.function.kill.kill_failed");
        }
    }
}

void sleepms(int ms)
{
    timespec sleeptime;
    sleeptime.tv_sec=ms/1000;
    sleeptime.tv_nsec=(ms-sleeptime.tv_sec*1000)*1000000;
    if(nanosleep(&sleeptime,&sleeptime)==-1) throw cubescript::error_key("runtime.function.server_sleep.returned_early");
}

static 
void log_daemon_error(const char * msg)
{
    FILE * file=fopen("logs/daemon.log","a");
    if(!file) return;
    fputs(msg,file);
    fputs("\n",file);
    fclose(file);
}

pid_t spawn_daemon(const std::string & filename,const std::vector<std::string> & args,const std::string & stdoutfile,const std::string & stderrfile)
{
    pid_t pid=fork();
    
    if(pid==0)
    {
        int maxfd=getdtablesize();
        for(int i=0; i<maxfd; i++) ::close(i);
        umask(0);
        setsid();
        
        if(open("/dev/null",O_RDONLY)==-1)
        {
            log_daemon_error("cannot open /dev/null");
            exit(1);
        }
        
        if(open(stdoutfile.c_str(),O_WRONLY | O_APPEND | O_CREAT,420)==-1)
        {
            log_daemon_error("cannot open stdout file");
            exit(1);
        }
        
        if(open(stderrfile.c_str(),O_WRONLY | O_APPEND | O_CREAT,420)==-1)
        {
            log_daemon_error("cannot open stderr file");
            exit(1);
        }
        
        char ** argv=new char * [args.size()+1];
        for(unsigned int i=0; i<args.size(); i++) argv[i]=newstring(args[i].c_str());
        argv[args.size()]=NULL;
        
        if(fork()==0)
        {
            execv(filename.c_str(),argv);
            
            std::ostringstream execfail;
            execfail<<filename<<" not executed."<<std::endl;
            log_daemon_error(execfail.str().c_str());
        }
        
        exit(1);
    }
    else
    {
        int status=0;
        waitpid(pid,&status,0);
        if(pid==-1) throw cubescript::error_key("runtime.function.daemon.fork_failed");
    }
    
    return pid;
}
