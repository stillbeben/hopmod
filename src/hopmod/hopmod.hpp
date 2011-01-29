#ifndef HOPMOD_HPP
#define HOPMOD_HPP

#include "signals.hpp"
#include "utils.hpp"
#include "events.hpp"

void init_hopmod();
void reload_hopmod();
void update_hopmod();
void restart_now();

// Scripting functions
void init_lua();
void shutdown_lua();
lua_State * get_lua_state();
lua::event_environment & event_listeners();

int get_player_id(const char * name, unsigned long ip);
void clear_player_ids();

// Scheduler Functions
void init_scheduler();
void update_scheduler(int);
void sched_callback(int (*)(void *),void *);
void sched_callback(int (*)(void *),void *, int);

extern unsigned int maintenance_frequency;
void set_maintenance_frequency(unsigned int);

enum{
    SHUTDOWN_NORMAL,
    SHUTDOWN_RESTART,
    SHUTDOWN_RELOAD
};

#endif

