
#ifndef NULL
#define NULL 0
#endif

#include <sys/time.h>

static time_t get_ticks()
{
    static bool set = false;
    static timeval started;
    
    if(!set)
    {
        if(gettimeofday(&started,NULL)!=0) return -1;
        set=true;
        return 0;
    }
    
    timeval current;
    if(gettimeofday(&current,NULL)!=0) return -1;
    
    return (1000*current.tv_sec+(current.tv_usec/1000))-(1000*started.tv_sec+(started.tv_usec/1000));
}
