
#include "sleep.hpp"
#include <boost/bind.hpp>
#include "get_ticks.cpp"

#include <iostream>

void sleep_service::run()
{
    while(!m_jobs.empty() && get_ticks() >= (time_t)m_jobs.top().m_launchtime)
    {
        sleep_job job=m_jobs.top();
        m_jobs.pop();
        job.m_code.apply(cubescript::args0(),job.m_domain);
    }
}

void sleep_service::new_job(std::list<std::string> & args,cubescript::domain * aDomain)
{
    sleep_job job;
    job.m_launchtime=get_ticks()+cubescript::functionN::pop_arg<unsigned long>(args);
    job.m_code=cubescript::functionN::pop_arg<cubescript::alias>(args);
    job.m_domain=aDomain->get_first_nontmp_domain();
    
    m_jobs.push(job);
}

void sleep_service::new_job(int countdown,const std::string & code,cubescript::domain * aDomain)
{
    
}

sleep_service::job::job(sleep_service::time_type ltime)
 :m_ltime(ltime)
{
    
}

sleep_service::job::~job()
{
    
}

inline const sleep_service::time_type & sleep_service::job::get_launch_time()const{return m_ltime;}

inline bool sleep_service::job::compare::operator()(const sleep_service::job & x,const sleep_service::job & y)const
{
    return x.get_launch_time() > y.get_launch_time();
}
