
#include "sleep.hpp"
#include <boost/bind.hpp>
#include "get_ticks.cpp"

void sleep_service::register_function(cubescript::domain * aDomain)
{
    static cubescript::functionV<void_> func_sleep(boost::bind(&sleep_service::new_job,this,_1,_2));
    aDomain->register_symbol("sleep",&func_sleep);
}

void sleep_service::run()
{
    while(!m_jobs.empty() && get_ticks() >= m_jobs.top().m_launchtime)
    {
        sleep_job job=m_jobs.top();
        job.m_code.apply(cubescript::args0(),job.m_domain);
        m_jobs.pop();
    }
}

void_ sleep_service::new_job(std::list<std::string> & args,cubescript::domain * aDomain)
{
    sleep_job job;
    job.m_launchtime=get_ticks()+cubescript::functionN::pop_arg<unsigned long>(args);
    job.m_code=cubescript::functionN::pop_arg<cubescript::alias>(args);
    job.m_domain=aDomain;
    
    m_jobs.push(job);
    
    return void_();
}
