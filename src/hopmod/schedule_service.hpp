/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_SCHEDULE_SERVICE_HPP
#define HOPMOD_SCHEDULE_SERVICE_HPP

#include <queue>
#include <boost/function.hpp>

class schedule_service
{
public:
    schedule_service():m_last_update(0){}
    
    template<typename NullaryFunction>
    void add_job(NullaryFunction func,int countdown,bool repeat = false)
    {
        job newJob(m_last_update + countdown,(repeat ? countdown : 0),func);
        m_jobs.push(newJob);
    }
    
    schedule_service & update(int totalmillis)
    {
        m_last_update = totalmillis;
        while(!m_jobs.empty() && totalmillis >= m_jobs.top().launch)
        {
            int status = m_jobs.top().function();
            if(m_jobs.top().interval && status != -1)
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
        boost::function0<int> function;
        template<typename NullaryFunction>
         job(int l,int i,NullaryFunction f):launch(l),interval(i),function(f){}
        bool operator<(const job & y)const{return launch > y.launch;}
    };
    
    int m_last_update;
    std::priority_queue<job> m_jobs;
};

#endif
