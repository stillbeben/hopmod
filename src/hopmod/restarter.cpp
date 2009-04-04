
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

extern int prog_argc;
extern char * const * prog_argv;

void exec_restarter()
{
    pid_t child = fork();
    bool parent = child != 0;
    
    if(parent)
    {
        int status;
        waitpid(child, &status, 0);
        
        umask(0);
        int maxfd = getdtablesize(); 
        for(int i=3; i<maxfd; i++) close(i);
        
        execv(prog_argv[0], prog_argv);
    }
}
