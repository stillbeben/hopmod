#ifndef HOPMOD_HPP
#define HOPMOD_HPP

#include "extapi.hpp"
#include "freqlimit.hpp"
#include "scripting.hpp"
#include "signals.hpp"
#include "banned_networks.hpp"
#include "timedbans_service.hpp"
#include "banlist.hpp"
#include "utils.hpp"
#include "masterserver_client.hpp"

#define GEOIP_NO_RESULT ""
bool geoip_supported();
bool load_geoip_database(const char *);
const char * ip_to_country(const char *);
const char * ip_to_country_code(const char *);

#include "env_fwd.hpp"

// Script Pipe Functions
void init_script_pipe();
bool open_script_pipe(const char *,int,fungu::script::env &);
void run_script_pipe_service(int);
void close_script_pipe();
void unlink_script_pipe();

// Script Socket Functions
bool script_socket_supported();
void init_script_socket();
bool open_script_socket(unsigned short, const char *);
void run_script_socket_service();
void close_script_socket();

// Restarter
void start_restarter();
void stop_restarter();

int get_player_id(const char * name, unsigned long ip);

void register_server_script_bindings(fungu::script::env &);

// Scheduler Functions
void init_scheduler();
void update_scheduler(int);
void cancel_all_scheduled();
void sched_callback(int (*)(void *),void *);

// Text Colouring Macros
#define GREEN "\f0"
#define BLUE "\f1"
#define YELLOW "\f2"
#define RED "\f3"
#define GREY "\f4"
#define MAGENTA "\f5"
#define ORANGE "\f6"

#endif
