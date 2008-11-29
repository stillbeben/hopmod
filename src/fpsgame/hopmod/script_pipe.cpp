/*
 *  Copyright (c) 2008 Graham Daws. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *      1. Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *          
 *      2. Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *         
 *      3. The names of the contributors and copyright holders must not be used
 *         to endorse or promote products derived from this software without
 *         specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "script_pipe.hpp"
#include "get_ticks.cpp"
#include <string>
#include <sstream>
#include <vector>
#include <boost/bind.hpp>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>

extern char **environ; //if this causes compile error then just remove it.

class script_pipe
{
public:
    script_pipe(const std::string & filename,
        const std::vector<std::string> & args,
        const std::string &,
        cubescript::domain *);
    ~script_pipe();
    void run();
    bool has_closed()const;
    void close();
    void offload_errors(std::ostream &);
    void kill();
private:
    void read_script_pipe(bool allow_write);
    void read_error_pipe();
    void run_code(bool allow_write);
    
    std::string m_filename;
    std::string m_fullname;
    int m_in;
    int m_out;
    int m_err;
    pid_t m_child_pid;
    cubescript::domain * m_domain;
    std::stringstream m_code;
    
    cubescript::cons::expression * m_expn;
    time_t m_expn_ctime;
    
    std::string m_stderr_errors;
    std::string m_local_errors;

    std::string m_onfinish;
};

class inside_pipe_error:public cubescript::error_key
{
public:
    inside_pipe_error(const char * key):cubescript::error_key(key){}
};

script_pipe::script_pipe(const std::string & filename,
    const std::vector<std::string> & argsv,
    const std::string & onfinish,
    cubescript::domain * aDomain)

 :  m_filename(filename),
    m_in(-1),
    m_out(-1),
    m_err(-1),
    m_domain(aDomain),
    m_expn(NULL),
    m_onfinish(onfinish)
{
    // read/write descriptors from the child's perspective
    int script_read[2];
    int script_write[2];
    int error_pipe[2];
    
    m_fullname=filename;
    for(unsigned int i=0; i<argsv.size(); i++) m_fullname+=" " + argsv[i];
    
    if( pipe(script_read)==-1 || pipe(script_write)==-1 || pipe(error_pipe)==-1) throw cubescript::error_key("runtime.function.script_pipe.pipe_failed");
    
    assert( script_read[0]>2 && script_read[1]>2 && script_write[0]>2 && script_write[1]>2 );
    
    pid_t pid=fork();
    
    if(pid==0)
    {
        ::close(script_read[1]);
        ::close(script_write[0]);
        ::close(error_pipe[0]);
        
        dup2(script_read[0],0);
        ::close(script_read[0]);
        
        dup2(script_write[1],1);
        ::close(script_write[1]);
        
        dup2(error_pipe[1],2);
        ::close(error_pipe[1]);
        
        int maxfd=getdtablesize();
        for(int i=3; i<maxfd; i++) ::close(i);
        
        char ** args=new char *[argsv.size()+2];
        args[0]=new char[filename.length()+1];
        strcpy(args[0],filename.c_str());
        for(unsigned int i=0; i<argsv.size(); i++)
        {
            const char * arg=argsv[i].c_str();
            args[i+1]=new char[argsv[i].length()+1];
            strcpy(args[i+1],arg);
        }
        args[argsv.size()+1]=NULL;
        
        execve(filename.c_str(),args,environ);
        
        const char * key;
        
        switch(errno)
        {
            case ENOENT: key="runtime.function.script_pipe.file_not_found"; break;
            case EACCES: key="runtime.function.script_pipe.permission_denied"; break;
            default: key="runtime.function.script_pipe.execve_failed";
        }
        
        throw inside_pipe_error(key);
    }
    else
    {
        m_child_pid=pid;
        
        ::close(script_write[1]);
        ::close(script_read[0]);
        ::close(error_pipe[1]);
        
        m_in=script_write[0];
        m_out=script_read[1];
        m_err=error_pipe[0];
        
        if( fcntl(m_in,F_SETFL,O_NONBLOCK)==-1 || 
            fcntl(m_out,F_SETFL,O_NONBLOCK)==-1 ||
            fcntl(m_err,F_SETFL,O_NONBLOCK)==-1 )
           throw cubescript::error_key("runtime.function.script_pipe.fcntl_failed");
    }

}

script_pipe::~script_pipe()
{
    close();
}

void script_pipe::run()
{
    if(m_expn && 
        script_pipe_service::get_exec_timeout() != -1 &&
        get_ticks() - m_expn_ctime >= script_pipe_service::get_exec_timeout())
    {
        m_local_errors += "Parsing expression on script pipe timed out.\r\n";
        delete m_expn;
        m_expn = NULL;
    }
    
    pollfd files[2];
    
    files[0].fd=m_in;
    files[0].events=POLLIN;
    files[0].revents=0;
    
    files[1].fd=m_err;
    files[1].events=POLLIN;
    files[1].revents=0;
    
    if(poll(files,2,0))
    {
        bool file_0_writable=!(files[0].revents & (POLLHUP | POLLERR));
        
        if(files[0].revents & POLLIN) read_script_pipe(file_0_writable);
        else if(files[1].revents & POLLIN) read_error_pipe();
        
        if( (files[0].revents | files[1].revents) & (POLLHUP | POLLERR) )
        {
            close();
            return;
        }
    }
    
}

void script_pipe::read_script_pipe(bool allow_write)
{
    char buffer[255];
    ssize_t readlen;
    
    do
    {
        readlen=read(m_in,buffer,sizeof(buffer)-1);
        
        if(readlen > 0)
        {
            buffer[readlen]='\0';
            m_code<<buffer;
        }
    }
    while(readlen>0);
    
    while(!m_code.eof()) run_code(allow_write);
    
    m_code.clear();
}

void script_pipe::read_error_pipe()
{
    char buffer[256];
    ssize_t bufferlen;
    
    do
    {
        bufferlen=read(m_err,buffer,sizeof(buffer)-1);
        
        if(bufferlen > 0)
        {
            buffer[bufferlen]='\0';
            
            if(buffer[bufferlen-1]=='\n')
            {
                if(buffer[bufferlen-2]=='\r') buffer[bufferlen-2]='\0';
                else buffer[bufferlen-1]='\0';
            }
            
            m_stderr_errors+=buffer;
        }
        
    }while(bufferlen>0);
}

void script_pipe::run_code(bool allow_write)
{
    bool del_expn = false;
    bool new_expn = m_expn == NULL;
    
    if(!m_expn)
    {
        m_expn = new cubescript::cons::expression;
        m_expn->set_domain(m_domain);
        m_expn_ctime = get_ticks();
    }
    
    std::string result;
    
    try
    {
        if(m_expn->parse(m_code))
        {
            char c = m_code.get();
            if(c=='\r') m_code.get();
            result = m_expn->eval();
            del_expn = true;
        }
        if(!m_expn->is_parsing()) del_expn = true;
    }
    catch(cubescript::error_key key)
    {
        m_local_errors += m_fullname + key.get_key() + "\r\n";
    }
    catch(cubescript::error_context * error)
    {
        m_local_errors += m_fullname + cubescript::format_error_report(error) + "\r\n";
        delete error;
    }
    
    result+="\n\n";
    if(allow_write) write(m_out,result.c_str(),result.length());
    
    if(del_expn)
    {
        delete m_expn;
        m_expn = NULL;
    }
}

bool script_pipe::has_closed()const
{
    return m_in==-1 || m_out==-1;
}

void script_pipe::close()
{
    if(m_in==-1 && m_out==-1 && m_child_pid==-1) return;
    
    ::close(m_in);
    ::close(m_out);
    
    m_in=-1;
    m_out=-1;
    
    int childstatus=0;
    wait4(m_child_pid,&childstatus,0,NULL);
    
    cubescript::alias_function finish_func(m_onfinish,m_domain);
    cubescript::arguments args;
    finish_func.run(args & childstatus);
    
    m_child_pid=-1;
}

void script_pipe::kill()
{
    ::close(m_in);
    ::close(m_out);
    ::kill(m_child_pid,SIGTERM);
    int childstatus=0;
    waitpid(m_child_pid,&childstatus,0);
    m_in=-1;
    m_out=-1;
    m_child_pid=-1;
}

void script_pipe::offload_errors(std::ostream & output)
{
    std::stringstream message;
    
    if(m_stderr_errors.length())
    {
        message<<m_fullname<<" [stderr]: "<<m_stderr_errors<<std::endl;
        output<<message.str();
        m_stderr_errors.clear();
    }
    
    message.seekp(0);
    
    if(m_local_errors.length())
    {
        message<<m_local_errors<<std::flush;
        output<<message.str();
        m_local_errors.clear();
    }
}

time_t script_pipe_service::sm_expr_timeout = -1;

script_pipe_service::script_pipe_service(std::ostream & output):m_error_output(output)
{
    
}

script_pipe_service::~script_pipe_service()
{
    for(std::list<script_pipe *>::iterator pipeIt=m_pipes.begin();pipeIt!=m_pipes.end(); pipeIt++) delete *pipeIt;
}

void script_pipe_service::register_function(cubescript::domain * aDomain)
{
    cubescript::symbol * create_pipe_function=
        new cubescript::function3<void,const std::string &,const std::vector<std::string> &,const std::string &>(
            boost::bind(&script_pipe_service::create_pipe,this,_1,_2,_3,aDomain));
    aDomain->register_symbol("script_pipe",create_pipe_function,cubescript::domain::ADOPT_SYMBOL);
    
    static cubescript::variable_ref<time_t> var_expr_timeout(sm_expr_timeout);
    aDomain->register_symbol("script_pipe_parse_timeout",&var_expr_timeout);
}

void script_pipe_service::run()
{
    for(std::list<script_pipe *>::iterator pipeIt=m_pipes.begin();pipeIt!=m_pipes.end(); pipeIt++)
    {
        (*pipeIt)->run();
        
        (*pipeIt)->offload_errors(m_error_output);
        
        if((*pipeIt)->has_closed())
        {
            delete *pipeIt;
            
            std::list<script_pipe *>::iterator tmpIt=pipeIt;
            pipeIt++;
            m_pipes.erase(tmpIt);
        }
    }
}

void script_pipe_service::shutdown()
{
    for(std::list<script_pipe *>::iterator pipeIt=m_pipes.begin();pipeIt!=m_pipes.end(); pipeIt++)
    {
        (*pipeIt)->kill();
        delete *pipeIt;
    }
    m_pipes.clear();
}

time_t script_pipe_service::get_exec_timeout()
{
    return sm_expr_timeout;
}

void script_pipe_service::create_pipe(const std::string & filename,const std::vector<std::string> & args,const std::string & onfinish,cubescript::domain * aDomain)
{
    try
    {
        script_pipe * newPipe=new script_pipe(filename,args,onfinish,aDomain);
        m_pipes.push_back(newPipe);
    }
    catch(const inside_pipe_error & e)
    {
        fputs(e.get_key(),stderr);
        fflush(stderr);
        exit(1);
    }
}
