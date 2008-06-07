#include "stopwatch.hpp"
#include <assert.h>

#ifndef NULL
#define NULL 0
#endif

#include <sys/time.h>
static 
time_t get_ticks()
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

stopwatch::stopwatch()
 :m_start(-1),m_stop(-1)
{
    
}

void stopwatch::start()
{
    m_start=get_ticks();
    m_stop=-1;
}

stopwatch & stopwatch::stop()
{
    m_stop=get_ticks();
    return *this;
}

void stopwatch::resume()
{
    m_start+=get_ticks()-m_stop;
    m_stop=-1;
}

stopwatch::milliseconds stopwatch::get_elapsed()const
{
    assert(m_stop!=(stopwatch::milliseconds)-1);
    return m_stop-m_start;
}

bool stopwatch::running()const
{
    return m_start!=(stopwatch::milliseconds)-1 && m_stop==(stopwatch::milliseconds)-1;
}
