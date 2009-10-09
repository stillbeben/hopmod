#ifndef HOPMOD_HPP
#define HOPMOD_HPP

extern "C"{
    #include <lua.h>
}

#include <fungu/script.hpp>

#include "extapi.hpp"
#include "freqlimit.hpp"
#include "signals.hpp"
#include "banned_networks.hpp"
#include "timedbans_service.hpp"
#include "utils.hpp"
#include "masterserver_client.hpp"
#include "lnetlib.hpp"
#include "lua/modules.hpp"

#define GEOIP_NO_RESULT ""
bool geoip_supported();
bool load_geoip_database(const char *);
const char * ip_to_country(const char *);
const char * ip_to_country_code(const char *);

// Scripting functions
void init_scripting();
void shutdown_scripting();
fungu::script::env & get_script_env();
std::string get_script_error_message(fungu::script::error_trace * errinfo);
void report_script_error(fungu::script::error_trace *);
void report_script_error(const char *);
void register_lua_function(lua_CFunction,const char *);
void unset_global(const char *);

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
void clear_player_ids();

void register_server_script_bindings(fungu::script::env &);

// Scheduler Functions
void init_scheduler();
void update_scheduler(int);
void cancel_free_scheduled();
void sched_callback(int (*)(void *),void *);

#include <vector>
#include <string>

// Player command functions and variables
std::vector<std::string> parse_player_command_line(const char *);
bool eval_player_command(int,const char *,const std::vector<std::string> &,fungu::script::env &);
bool process_player_command(int,const char *);
extern char command_prefix;
extern bool using_command_prefix;

// Text Colouring Macros
#define GREEN "\f0"
#define BLUE "\f1"
#define YELLOW "\f2"
#define RED "\f3"
#define GREY "\f4"
#define MAGENTA "\f5"
#define ORANGE "\f6"

#endif
