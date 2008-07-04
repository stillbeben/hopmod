
#ifndef SCHEDULE_HPP
#define SCHEDULE_HPP

#include <list>
#include <queue>

class schedule_service
{
public:
    typedef unsigned long time_type;
    class job;
    
    void run_service();
private:
    void new_job(std::list<std::string> & args,cubescript::domain * d);
    void new_job(int,const std::string &,cubescript::domain *);
    std::priority_queue<job,std::vector<job>,job::compare> m_jobs;
};

class schedule_service::job
{
public:
    job(time_type);
    virtual ~job();
    virtual job * clone(time_type)const=0;
    const time_type & get_launch_time()const;
    class compare;
private:
    virtual void run_job()=0;
    time_type m_ltime;
};

class schedule_service::job::compare
{
public:
    bool operator()(const sleep_service::job &,const sleep_service::job &)const;
};

#endif
