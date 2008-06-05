
#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <cubescript.hpp>
#include <list>
#include <queue>

struct sleep_job
{
    unsigned long m_launchtime;
    cubescript::alias m_code;
    cubescript::domain * m_domain;
};

struct cmp_sleep_jobs
{
    bool operator()(const sleep_job & x,const sleep_job & y)
    {
        return x.m_launchtime > y.m_launchtime;
    }
};

class sleep_service
{
public:
    void register_function(cubescript::domain *);
    void run();
private:
    void_ new_job(std::list<std::string> & args,cubescript::domain * d);
    std::priority_queue<sleep_job,std::vector<sleep_job>,cmp_sleep_jobs> m_jobs;
};

#endif
