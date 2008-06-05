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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>

#define MAX(a,b) (a>b?a:b)

const char * filename_pipe;

void cleanup()
{
    unlink(filename_pipe);
}

static void cleanup_from_signal(int i)
{
    cleanup();
    exit(-1);
}

int permissions(const char * code)
{
    if(strlen(code)!=3) return -1;
    int user=code[0]-'0';
    int group=code[1]-'0';
    int others=code[2]-'0';
    if( user > 7 || group > 7 || others > 7 ) return -1;
    return (user << 6) | (group << 3) | others;
}

int main(int argc,const char ** argv)
{
    FILE * error=fdopen(2,"a");
    FILE * script_read=fdopen(0,"r");
    FILE * script_write=fdopen(1,"w");
    
    if(argc!=3) return 1;
    const char * filename=argv[1];
    int perms=permissions(argv[2]);
    
    if(perms == -1)
    {
        fprintf(error,"invalid permission code\n");
        return 1;
    }
    
    umask(0);
    
    if(mkfifo(filename,perms)==-1)
    {
        switch(errno)
        {
            case EEXIST: fprintf(error,"file already in use\n"); break;
            default: fprintf(error,"unable to create named pipe (errno %i)\n",errno);
        }
        return 1;
    }
    
    filename_pipe=filename;
    
    struct sigaction terminate_action;
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_handler=cleanup_from_signal;
    terminate_action.sa_flags=0;
    
    sigaction(SIGINT,&terminate_action,NULL);
    sigaction(SIGTERM,&terminate_action,NULL);
    sigaction(SIGPIPE,&terminate_action,NULL);
    
    while(!feof(script_write))
    {
        FILE * pipe=fopen(filename,"r");
        
        if(!pipe)
        {
            fprintf(error,"unable to open file (errno %i)\n",errno);
            return 1;
        }
        
        while(!feof(script_write) && !feof(pipe) )
        {
            char line[128];
            line[0]='\0';
            if(!fgets(line,sizeof(line),pipe)) break;
            
            fputs(line,script_write);
            fflush(script_write);
            
            char null[128];
            fgets(null,sizeof(null),script_read);
        }
        
        fclose(pipe);
    }
    
    
    
    cleanup();
    
    return 0;
}
