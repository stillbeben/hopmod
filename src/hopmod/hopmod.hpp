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
void log_event_error(const char * event_id, const char * error_message);

int get_player_id(const char * name, unsigned long ip);
void clear_player_ids();

// Scheduler Functions
void init_scheduler();
void update_scheduler(int);
void sched_callback(int (*)(void *),void *);
void sched_callback(int (*)(void *),void *, int);
void cancel_timer(int);
namespace lua{
int sleep(lua_State *);
int interval(lua_State *L);
} //namespace lua

extern unsigned int maintenance_frequency;
void set_maintenance_frequency(unsigned int);

enum{
    SHUTDOWN_NORMAL,
    SHUTDOWN_RESTART,
    SHUTDOWN_RELOAD
};

#endif

