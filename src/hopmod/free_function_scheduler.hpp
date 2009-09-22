/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_FREE_FUNCTION_SCHEDULER_HPP
#define HOPMOD_FREE_FUNCTION_SCHEDULER_HPP

#include <queue>
#include <boost/function.hpp>

class free_function_scheduler
{
public:
    free_function_scheduler()
    :m_last_update(0)
    {
        
    }
    
    template<typename NullaryFunction>
    void schedule(NullaryFunction func, int countdown, bool repeat = false)
    {
        job new_job;
        new_job.function = func;
        new_job.launch = m_last_update + countdown;
        new_job.interval = countdown;
        new_job.repeat = repeat;
        
        m_jobs.push(new_job);
    }
    
    free_function_scheduler & update(int totalmillis)
    {
        m_last_update = totalmillis;
        
        while(!m_jobs.empty() && totalmillis >= m_jobs.top().launch)
        {
            int status = m_jobs.top().function();
            if(m_jobs.top().repeat && status != -1)
            {
                job resched = m_jobs.top();
                resched.launch += resched.interval;
                m_jobs.push(resched);
            }
            m_jobs.pop();
        }
        
        return *this;
    }
    
    void cancel_all()
    {
        while(!m_jobs.empty()) m_jobs.pop();
    }
private:
    
    struct job
    {
        int launch;
        int interval;
        bool repeat;
        boost::function0<int> function;
        
        bool operator<(const job & y)const
        {
            return launch > y.launch;
        }
    };
    
    int m_last_update;
    std::priority_queue<job> m_jobs;
};

#endif
