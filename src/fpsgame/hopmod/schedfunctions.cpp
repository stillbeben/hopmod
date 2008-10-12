
#include "schedfunctions.hpp"
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace cubescript
{

inline void sleep_job_runner(const std::string & code,domain * aDomain){exec_block(code,aDomain);}
inline void schedule_sleep_function(schedule_service * scheduler,domain * aDomain,int countdown,const std::string & code){scheduler->schedule(boost::bind(sleep_job_runner,code,aDomain),countdown);}

void schedule_interval_function(schedule_service *,domain *,int,const std::string &);

void interval_job_runner(const std::string & code,domain * aDomain,schedule_service * scheduler,int countdown)
{
    domain local(aDomain,domain::TEMPORARY_DOMAIN);
    cubescript::nullary_setter stop;
    local.register_symbol("stop",&stop);
    exec_block(code,&local);
    if(!stop.is_set()) schedule_interval_function(scheduler,aDomain,countdown,code);
}

inline void schedule_interval_function(schedule_service * scheduler,domain * aDomain,int countdown,const std::string & code)
{
    scheduler->schedule(boost::bind(interval_job_runner,code,aDomain,scheduler,countdown),countdown);
}

void register_schedule_functions(schedule_service * scheduler,domain * aDomain)
{
    aDomain->register_symbol("sleep",new function2<void,int,const std::string &>(boost::bind(schedule_sleep_function,scheduler,aDomain,_1,_2)),domain::ADOPT_SYMBOL);
    aDomain->register_symbol("interval",new function2<void,int,const std::string &>(boost::bind(schedule_interval_function,scheduler,aDomain,_1,_2)),domain::ADOPT_SYMBOL);
}

} //namespace cubescript
