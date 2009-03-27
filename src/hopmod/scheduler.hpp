#ifndef HOPMOD_SCHEDULER_HPP
#define HOPMOD_SCHEDULER_HPP

#include "env_fwd.hpp"

void init_scheduler();
void update_scheduler(int);
void cancel_all_scheduled();
void sched_callback(int (*)(void *),void *);

#endif
