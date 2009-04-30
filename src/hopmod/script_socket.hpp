#ifndef HOPMOD_SCRIPT_SOCKET_HPP
#define HOPMOD_SCRIPT_SOCKET_HPP

#include "env_fwd.hpp"

bool script_socket_supported();
void init_script_socket();
bool open_script_socket(unsigned short, const char *);
void run_script_socket_service();
void close_script_socket();

#endif
